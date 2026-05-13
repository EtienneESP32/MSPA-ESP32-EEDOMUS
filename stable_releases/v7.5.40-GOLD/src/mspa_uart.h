#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <atomic>
#include <cmath>
#include <deque>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <vector>

namespace esphome {
namespace mspa {

static const char *const TAG = "mspa_uart";

struct EedomusRequest {
  int periph_id;
  float value;
  bool is_float;
};

class MSPAUartComponent : public Component {
public:
  MSPAUartComponent(uart::UARTComponent *spa, uart::UARTComponent *kbd)
      : uart_spa_(spa), uart_kbd_(kbd) {}

  void set_temp_sensor(sensor::Sensor *s) { temp_sensor_ = s; }
  void set_setpoint_sensor(number::Number *s) { setpoint_sensor_ = s; }
  void set_filter_alert_sensor(binary_sensor::BinarySensor *s) {
    filter_alert_sensor_ = s;
  }
  void set_f_switch(switch_::Switch *s) { f_switch_ = s; }
  void set_h_switch(switch_::Switch *s) { h_switch_ = s; }
  void set_u_switch(switch_::Switch *s) { u_switch_ = s; }
  void set_b_select(select::Select *s) { b_select_ = s; }
  void set_link_sensor(binary_sensor::BinarySensor *s) { link_sensor_ = s; }
  void set_kbd_link_sensor(binary_sensor::BinarySensor *s) {
    kbd_link_sensor_ = s;
  }
  void set_lock_switch(switch_::Switch *s) { lock_switch_ = s; }
  void set_lock(bool lock) { lock_kbd_.store(lock); }
  void set_eedomus_callback(std::function<void(int, float, bool)> &&callback) {
    eedomus_callback_ = std::move(callback);
  }
  void set_http_busy(bool busy) { http_busy_.store(busy); }
  bool is_eedomus_enabled() { return eedomus_enabled_.load(); }
  void set_eedomus_enabled(bool enabled) { eedomus_enabled_.store(enabled); }
  void reset_filter_alert() { retry_reset_ = 10; }
  void control_lock(bool v) { 
    lock_kbd_.store(v); 
    if (lock_switch_) lock_switch_->publish_state(v);
  }

  // --- VARIABLES ATOMIQUES (v7.5.26) ---
  std::atomic<bool> target_f_{false}, target_h_{false}, target_u_{false};
  std::atomic<int> target_b_{0};
  std::atomic<uint8_t> retry_f_{0}, retry_h_{0}, retry_u_{0}, retry_b_{0},
      retry_reset_{0};
  std::atomic<bool> real_f_{false}, real_h_{false}, real_u_{false};
  std::atomic<int> real_b_{0};
  std::atomic<float> real_temp_{0.0f}, real_setpoint_{0.0f};
  std::atomic<bool> is_ready_{false}, is_sync_{false};
  std::atomic<bool> eedomus_enabled_{false}, http_busy_{false};
  std::atomic<bool> lock_kbd_{false};
  std::atomic<bool> physical_f_on_{false}, physical_h_on_{false};
  std::atomic<bool> is_blinking_f_{false}, is_blinking_h_{false};
  std::atomic<bool> bus_f_{false}, bus_h_{false}, bus_u_{false};
  std::atomic<int> bus_b_{0};
  std::atomic<uint32_t> last_on_f_{0}, last_on_h_{0}, last_on_u_{0};
  std::atomic<uint32_t> last_spa_ms_{0}, last_kbd_ms_{0};
  std::atomic<uint32_t> last_f_cmd_ms_{0}, last_h_cmd_ms_{0}, last_u_cmd_ms_{0},
      last_b_cmd_ms_{0};

  void enqueue_eedomus(int p_id, float val, bool is_f, bool force = false) {
    if (!eedomus_enabled_.load())
      return;

    // --- DÉDOUBLONNAGE INTELLIGENT (Nettoie les deux files) ---
    auto clear_from = [&](std::deque<EedomusRequest> &q) {
      for (auto it = q.begin(); it != q.end(); ++it) {
        if (it->periph_id == p_id) {
          q.erase(it);
          return;
        }
      }
    };
    clear_from(queue_actions_);
    clear_from(queue_status_);

    if (force) {
      if (queue_actions_.size() > 10)
        queue_actions_.pop_front();
      queue_actions_.push_back({p_id, val, is_f});
    } else {
      if (queue_status_.size() > 15)
        queue_status_.pop_front();
      queue_status_.push_back({p_id, val, is_f});
    }
  }

  bool inject_cmd(uint8_t id, uint8_t val) {
    uint8_t b[4] = {0xA5, id, val, (uint8_t)(0xA5 + id + val)};
    ESP_LOGI(TAG, "Sniper: Injecting CMD %02X Val %02X", id, val);
    if (xSemaphoreTakeRecursive(uart_mutex_, pdMS_TO_TICKS(50))) {
      uart_spa_->write_array(b, 4);
      xSemaphoreGiveRecursive(uart_mutex_);
      return true;
    }
    return false;
  }

  float get_current_temp() { return real_temp_.load(); }

  void run_watchdog() {
    uint32_t now = millis();
    if (link_sensor_)
      link_sensor_->publish_state(now - last_spa_ms_.load() < 3500);
    if (kbd_link_sensor_)
      kbd_link_sensor_->publish_state(now - last_kbd_ms_.load() < 3500);
    if (filter_alert_sensor_) {
      bool alert = (is_blinking_f_ && !physical_f_on_.load());
      if (alert != last_alert_val_) {
        last_alert_val_ = alert;
        filter_alert_sensor_->publish_state(alert);
      }
    }
  }

  void setup() override {
    ESP_LOGI(TAG, "MSPA v7.5.24-BIBLE-ULTRA Starting (Master Sanctuary v2)...");
    uart_mutex_ = xSemaphoreCreateRecursiveMutex();
    if (uart_mutex_ != NULL) {
      xTaskCreatePinnedToCore(MSPAUartComponent::uart_task_static,
                              "mspa_uart_task", 8192, this, 15,
                              &uart_task_handle_, 1);
    }
    is_ready_ = true;
  }

  void loop() override {
    uint32_t now = millis();

    // --- FAIL-SAFE HTTP (Protection contre les requêtes gelées) ---
    if (http_busy_ && (now - last_http_start_ms_ > 10000)) {
      ESP_LOGW(TAG,
               "Fail-Safe: Libération forcée du verrou HTTP (Timeout 10s)");
      http_busy_ = false;
    }

    // Watchdog Diag (Core 0)
    if (now - last_watchdog_ > 2000) {
      last_watchdog_ = now;
      run_watchdog();
    }

    // Eedomus Dispatcher (Core 0 - Stratégie Priorisée FIFO)
    if (eedomus_enabled_.load() && !http_busy_.load() &&
        (now - last_http_ms_ > 5000)) {
      
      // --- SOCKET GUARD ---
      uint32_t free_heap = esp_get_free_heap_size();
      if (free_heap < 60000) {
        static uint32_t last_heap_warn = 0;
        if (now - last_heap_warn > 60000) {
          ESP_LOGW(TAG, "Socket Guard: Heap critique (%u), suspension du push", free_heap);
          last_heap_warn = now;
        }
      } else {
        EedomusRequest req;
        bool found = false;

        if (!queue_actions_.empty()) {
          req = queue_actions_.front();
          queue_actions_.pop_front();
          found = true;
        } else if (!queue_status_.empty()) {
          req = queue_status_.front();
          queue_status_.pop_front();
          found = true;
        }

        if (found) {
          last_http_ms_ = now;
          last_http_start_ms_ = now;
          if (eedomus_callback_) {
            set_http_busy(true);
            eedomus_callback_(req.periph_id, req.value, req.is_float);
          }
        }
      }
    }
  }

  // --- ACTIONS MAITRES (Reset Logic) ---
  void control_filtration(bool state) {
    target_f_ = state;
    retry_f_ = 10;
    is_blinking_f_ = false;
    last_f_change_ms_ = millis() - 2000;
    last_f_cmd_ms_ = millis();
    if (f_switch_) f_switch_->publish_state(state); // Optimistic UI
  }
  void control_heating(bool state) {
    target_h_ = state;
    retry_h_ = 10;
    is_blinking_h_ = false;
    last_h_change_ms_ = millis() - 2000;
    last_h_cmd_ms_ = millis();
    if (h_switch_) h_switch_->publish_state(state); // Optimistic UI
  }
  void control_uvc(bool state) {
    target_u_ = state;
    retry_u_ = 10;
    last_u_change_ms_ = millis() - 2000;
    last_u_cmd_ms_ = millis();
    if (u_switch_) u_switch_->publish_state(state); // Optimistic UI
  }
  void control_bubbles(int level) {
    target_b_ = level;
    retry_b_ = 10;
    last_b_change_ms_ = millis() - 2000;
    last_b_cmd_ms_ = millis();
    if (b_select_) {
        b_select_->publish_state((level == 1) ? "Niveau1" : 
                                 (level == 2) ? "Niveau2" : 
                                 (level == 3) ? "Niveau3" : "Arret");
    }
  }

protected:
  uart::UARTComponent *uart_spa_;
  uart::UARTComponent *uart_kbd_;
  TaskHandle_t uart_task_handle_;
  SemaphoreHandle_t uart_mutex_;

  sensor::Sensor *temp_sensor_{nullptr};
  number::Number *setpoint_sensor_{nullptr};
  switch_::Switch *f_switch_{nullptr}, *h_switch_{nullptr}, *u_switch_{nullptr},
      *lock_switch_{nullptr};
  select::Select *b_select_{nullptr};
  binary_sensor::BinarySensor *filter_alert_sensor_{nullptr},
      *link_sensor_{nullptr}, *kbd_link_sensor_{nullptr};

  uint32_t last_watchdog_{0};
  uint32_t last_http_ms_{0}, last_http_start_ms_{0};
  uint32_t last_f_change_ms_{0}, last_h_change_ms_{0}, last_u_change_ms_{0},
      last_b_change_ms_{0}, last_set_change_ms_{0};
  std::deque<EedomusRequest> queue_actions_;
  std::deque<EedomusRequest> queue_status_;
  std::function<void(int, float, bool)> eedomus_callback_;

  uint8_t kbd_buf_[10], kbd_idx_{0};
  uint8_t spa_buf_[10], spa_idx_{0};
  bool last_alert_val_{false};

  // Ghosting State
  bool lp_f_{false}, lp_h_{false};
  uint32_t lc_f_{0}, lc_h_{0};

  static void uart_task_static(void *pvParameters) {
    MSPAUartComponent *component =
        static_cast<MSPAUartComponent *>(pvParameters);
    while (true) {
      component->uart_task();
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }

  void uart_task() {
    int quota = 64;
    while (uart_spa_->available() && quota-- > 0) {
      uint8_t c;
      if (uart_spa_->read_byte(&c)) {
        // --- RELAIS SANS MUTEX (Priorité Absolue) ---
        uart_kbd_->write_byte(c);
        process_machine(c, true);
      }
    }
    quota = 64;
    while (uart_kbd_->available() && quota-- > 0) {
      uint8_t c;
      if (uart_kbd_->read_byte(&c))
        process_machine(c, false);
    }

    // Sniper (Lock minimal uniquement sur l'écriture vers SPA)
    if (is_sync_.load()) {
      if (retry_f_.load() > 0) {
        if (inject_cmd(0x01, target_f_.load() ? 0x01 : 0x00))
          retry_f_--;
      } else if (retry_h_.load() > 0) {
        if (inject_cmd(0x01, target_h_.load() ? 0x01 : 0x00))
          retry_h_--;
      } else if (retry_u_.load() > 0) {
        if (inject_cmd(0x01, target_u_.load() ? 0x01 : 0x00))
          retry_u_--;
      } else if (retry_b_.load() > 0) {
        if (inject_cmd(0x03, (uint8_t)target_b_.load()))
          retry_b_--;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(1));
  }

  void process_machine(uint8_t c, bool from_spa) {
    uint8_t *idx_ptr = from_spa ? &spa_idx_ : &kbd_idx_;
    uint8_t *buf = from_spa ? spa_buf_ : kbd_buf_;
    uint32_t now = millis();

    if (*idx_ptr > 0) {
      uint32_t last = from_spa ? last_spa_ms_.load() : last_kbd_ms_.load();
      if (now - last > 100)
        *idx_ptr = 0;
    }

    if (from_spa)
      last_spa_ms_.store(now);
    else
      last_kbd_ms_.store(now);

    if (*idx_ptr == 0) {
      if (c == 0xA5 || c == 0x00) {
        buf[0] = c;
        *idx_ptr = 1;
      }
      return;
    }

    if (*idx_ptr < 9) {
      buf[(*idx_ptr)++] = c;
      int len = (buf[0] == 0x00) ? 5 : ((buf[1] == 0x1B) ? 5 : 4);
      if (*idx_ptr == len) {
        bool valid = false;
        if (buf[0] == 0xA5) {
          uint8_t cs = 0;
          for (int i = 0; i < len - 1; i++)
            cs += buf[i];
          valid = (buf[len - 1] == cs);
        } else if (buf[0] == 0x00) {
          uint8_t cs = (uint8_t)(buf[1] + buf[2] + buf[3] - 0x11);
          valid = (buf[len - 1] == cs);
        }
        if (valid) {
          if (!from_spa && (!lock_kbd_.load() || buf[1] == 0x0D)) {
            // Relais Clavier -> SPA (Mutex nécessaire pour ne pas entrer en
            // collision avec Sniper)
            if (xSemaphoreTakeRecursive(uart_mutex_, pdMS_TO_TICKS(10))) {
              uart_spa_->write_array(buf, len);
              xSemaphoreGiveRecursive(uart_mutex_);
            }
          }
          handle_frame(buf[1], buf[2], (len == 5 ? buf[3] : 0), from_spa);
        }
        *idx_ptr = 0;
      }
    }
  }

  void handle_frame(uint8_t id, uint8_t d1, uint8_t d2, bool from_spa) {
    uint32_t now = millis();

    if (!from_spa) {
      // PHASE 1: NON-DICTATURE (Priorité Utilisateur)
      target_f_ = real_f_.load();
      target_h_ = real_h_.load();
      target_u_ = real_u_.load();
      target_b_ = (int)real_b_.load();
      retry_f_ = 0; retry_h_ = 0; retry_u_ = 0; retry_b_ = 0;
      return;
    }

    if (id == 0x08) {
      physical_f_on_ = (d1 & 0x01);
      physical_h_on_ = (d1 & 0x02);
    }
    if (id == 0x1A) {
      bool pf = (d1 & 0x01), ph = (d1 & 0x02), pu = (d1 & 0x04);

      // Détection du clignotement (Nécessaire avant l'état final)
      auto detect_ghost = [&](bool current, bool &last_state,
                              uint32_t &last_change,
                              std::atomic<bool> &is_ghost) {
        if (current != last_state) {
          uint32_t dt = now - last_change;
          if (dt > 50 && dt < 1500)
            is_ghost = true;
          last_change = now;
          last_state = current;
        }
        if (now - last_change > 3500)
          is_ghost = false;
      };
      detect_ghost(pf, lp_f_, lc_f_, is_blinking_f_);
      detect_ghost(ph, lp_h_, lc_h_, is_blinking_h_);

      // PHASE 3: DÉTECTEUR D'ENVELOPPE (Latching 3s)
      if (pf) last_on_f_ = now;
      if (ph) last_on_h_ = now;
      if (pu) last_on_u_ = now;

      bool env_f = (now - last_on_f_.load() < 3000);
      bool env_h = (now - last_on_h_.load() < 3000);
      bool env_u = (now - last_on_u_.load() < 3000);

      // --- RÈGLE HYBRIDE: PRIORITÉ PHYSIQUE SI CLIGNOTEMENT ---
      bool final_f = is_blinking_f_.load() ? physical_f_on_.load() : env_f;
      bool final_h = is_blinking_h_.load() ? physical_h_on_.load() : env_h;
      bool final_u = env_u;

      // Mise à jour de l'UI (L'enveloppe stabilise, le physique prime en alerte)
      update_ui_switch(now, final_f, real_f_, last_f_change_ms_, f_switch_, false);
      update_ui_switch(now, final_h, real_h_, last_h_change_ms_, h_switch_, false);
      update_ui_switch(now, final_u, real_u_, last_u_change_ms_, u_switch_, false);

      bus_f_ = env_f;
      bus_h_ = env_h;
      bus_u_ = env_u;

    }
    if (id == 0x1B) {
      int pb = d1;
      bus_b_ = pb;
      if (pb != real_b_.load()) {
        if (now - last_b_change_ms_ > 1500) {
          real_b_ = pb;
          if (b_select_)
            b_select_->publish_state((pb == 1)   ? "Niveau1"
                                     : (pb == 2) ? "Niveau2"
                                     : (pb == 3) ? "Niveau3"
                                                 : "Arret");
        }
      } else
        last_b_change_ms_ = now;
      float ps = 30.0f + (int8_t)d2;
      if (std::abs(ps - real_setpoint_.load()) > 0.1f) {
        if (now - last_set_change_ms_ > 1500) {
          real_setpoint_ = ps;
          ESP_LOGI(TAG, "Sync: Setpoint -> %.1f", ps);
          if (setpoint_sensor_)
            setpoint_sensor_->publish_state(ps);
        }
      } else
        last_set_change_ms_ = now;
    }
    if (id == 0x06) {
      float nt = d1 / 2.0f;
      if (std::abs(nt - real_temp_.load()) > 0.1f) {
        real_temp_ = nt;
        ESP_LOGI(TAG, "Sync: Water Temp -> %.1f", nt);
        if (temp_sensor_)
          temp_sensor_->publish_state(nt);
      }
    }
    if (id == 0x06 || id == 0x1B || id == 0x1A) {
      check_sniper(target_f_, bus_f_, retry_f_, 0x02);
      check_sniper(target_h_, bus_h_, retry_h_, 0x01);
      check_sniper(target_u_, bus_u_, retry_u_, 0x19);
      if (target_b_.load() != bus_b_.load() && retry_b_.load() > 0) {
        if (inject_cmd(0x03, (uint8_t)target_b_.load()))
          retry_b_--;
      }
      if (retry_reset_.load() > 0) {
        if (inject_cmd(0x02, 0x01))
          retry_reset_--;
      }

      // --- PHASE 4: ARBITRAGE DIAMOND (10s timeout avant Revert) ---
      uint32_t now = millis();
      
      // Filtration Revert
      if (retry_f_.load() == 0 && last_f_cmd_ms_.load() > 0 && (now - last_f_cmd_ms_.load() > 10000) && (now - last_f_cmd_ms_.load() < 60000)) {
          if (target_f_.load() != bus_f_.load()) {
              ESP_LOGW(TAG, "Diamond: Commande Filtration échouée. Revert.");
              target_f_ = bus_f_.load();
              if (f_switch_) f_switch_->publish_state(target_f_.load());
              last_f_cmd_ms_ = 0;
          }
      }
      // Chauffage Revert
      if (retry_h_.load() == 0 && last_h_cmd_ms_.load() > 0 && (now - last_h_cmd_ms_.load() > 10000) && (now - last_h_cmd_ms_.load() < 60000)) {
          if (target_h_.load() != bus_h_.load()) {
              ESP_LOGW(TAG, "Diamond: Commande Chauffage échouée. Revert.");
              target_h_ = bus_h_.load();
              if (h_switch_) h_switch_->publish_state(target_h_.load());
              last_h_cmd_ms_ = 0;
          }
      }
      // UVC Revert
      if (retry_u_.load() == 0 && last_u_cmd_ms_.load() > 0 && (now - last_u_cmd_ms_.load() > 10000) && (now - last_u_cmd_ms_.load() < 60000)) {
          if (target_u_.load() != bus_u_.load()) {
              ESP_LOGW(TAG, "Diamond: Commande UVC échouée. Revert.");
              target_u_ = bus_u_.load();
              if (u_switch_) u_switch_->publish_state(target_u_.load());
              last_u_cmd_ms_ = 0;
          }
      }
      // Bulles Revert
      if (retry_b_.load() == 0 && last_b_cmd_ms_.load() > 0 && (now - last_b_cmd_ms_.load() > 10000) && (now - last_b_cmd_ms_.load() < 60000)) {
          if (target_b_.load() != bus_b_.load()) {
              ESP_LOGW(TAG, "Diamond: Commande Bulles échouée. Revert.");
              target_b_ = bus_b_.load();
              int pb = bus_b_.load();
              if (b_select_) b_select_->publish_state((pb==1)?"Niveau1":(pb==2)?"Niveau2":(pb==3)?"Niveau3":"Arret");
              last_b_cmd_ms_ = 0;
          }
      }
    }
  }

  void update_ui_switch(uint32_t now, bool current, std::atomic<bool> &real,
                        uint32_t &last_change, switch_::Switch *sw,
                        bool force) {
    if (current != real.load()) {
      if (now - last_change > 1500 || force) {
        real = current;
        last_change = now;
        ESP_LOGI(TAG, "UI: %s -> %s (Filter: ENV)",
                 sw ? sw->get_name().c_str() : "Switch", current ? "ON" : "OFF",
                 force ? "GHOST" : "ENV");
        if (sw)
          sw->publish_state(current);
      }
    } else
      last_change = now;
  }

  void check_sniper(std::atomic<bool> &target, std::atomic<bool> &bus,
                    std::atomic<uint8_t> &retry, uint8_t cmd) {
    if (target.load() != bus.load() && retry > 0) {
      if (inject_cmd(cmd, target.load() ? 0x01 : 0x00))
        retry--;
    }
  }
};

} // namespace mspa
} // namespace esphome
