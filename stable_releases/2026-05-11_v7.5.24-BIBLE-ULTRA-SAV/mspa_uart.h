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
#include <functional>
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
  void set_lock(bool lock) { lock_ = lock; }
  void set_eedomus_callback(std::function<void(int, float, bool)> &&callback) {
    eedomus_callback_ = std::move(callback);
  }
  void set_http_busy(bool busy) { http_busy_ = busy; }
  void set_eedomus_enabled(bool enabled) { eedomus_enabled_ = enabled; }

  // --- VARIABLES ATOMIQUES ---
  std::atomic<bool> target_f_{false}, target_h_{false}, target_u_{false};
  std::atomic<int> target_b_{0}, real_b_{0};
  std::atomic<bool> real_f_{false}, real_h_{false}, real_u_{false};
  std::atomic<float> real_temp_{0}, real_setpoint_{0};
  std::atomic<bool> is_ready_{false};
  std::atomic<uint8_t> retry_f_{0}, retry_h_{0}, retry_u_{0}, retry_b_{0};
  std::atomic<bool> physical_f_on_{false}, physical_h_on_{false};
  std::atomic<bool> is_blinking_f_{false}, is_blinking_h_{false};
  std::atomic<bool> bus_f_{false}, bus_h_{false}, bus_u_{false};
  std::atomic<int> bus_b_{0};

  void enqueue_eedomus(int p_id, float val, bool is_f, bool force = false) {
    if (!eedomus_enabled_) return;
    if (eedomus_queue_.size() > 20) eedomus_queue_.pop_back();
    if (force) eedomus_queue_.push_front({p_id, val, is_f});
    else eedomus_queue_.push_back({p_id, val, is_f});
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
    if (link_sensor_) link_sensor_->publish_state(now - last_spa_ms_ < 3500);
    if (kbd_link_sensor_) kbd_link_sensor_->publish_state(now - last_kbd_ms_ < 3500);
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
                            "mspa_uart_task", 8192, this, 5, &uart_task_handle_, 1);
    }
    is_ready_ = true;
  }

  void loop() override {
    uint32_t now = millis();
    if (http_busy_ && (now - last_http_start_ms_ > 30000)) http_busy_ = false;
    if (now - last_watchdog_ > 2000) { last_watchdog_ = now; run_watchdog(); }
    if (eedomus_enabled_ && !http_busy_ && !eedomus_queue_.empty() && (now - last_http_ms_ > 15000)) {
      EedomusRequest req = eedomus_queue_.front(); eedomus_queue_.pop_front();
      last_http_ms_ = now; last_http_start_ms_ = now;
      if (eedomus_callback_) { set_http_busy(true); eedomus_callback_(req.periph_id, req.value, req.is_float); }
    }
  }

  // --- ACTIONS MAITRES (Reset Logic) ---
  void control_filtration(bool state) {
    if (target_f_.load() != state) {
      target_f_ = state; retry_f_ = 10;
      is_blinking_f_ = false; // Reset Ghost
      last_f_change_ms_ = millis() - 2000; // Prime Silk Filter
      lc_f_ = millis();
    }
  }
  void control_heating(bool state) {
    if (target_h_.load() != state) {
      target_h_ = state; retry_h_ = 10;
      is_blinking_h_ = false; // Reset Ghost
      last_h_change_ms_ = millis() - 2000; // Prime Silk Filter
      lc_h_ = millis();
    }
  }
  void control_uvc(bool state) {
    if (target_u_.load() != state) {
      target_u_ = state; retry_u_ = 10;
      last_u_change_ms_ = millis() - 2000; // Prime Silk Filter
    }
  }
  void control_bubbles(int level) {
    if (target_b_.load() != level) {
      target_b_ = level; retry_b_ = 10;
      last_b_change_ms_ = millis() - 2000; // Prime Silk Filter
    }
  }

protected:
  uart::UARTComponent *uart_spa_;
  uart::UARTComponent *uart_kbd_;
  TaskHandle_t uart_task_handle_;
  SemaphoreHandle_t uart_mutex_;

  sensor::Sensor *temp_sensor_{nullptr};
  number::Number *setpoint_sensor_{nullptr};
  switch_::Switch *f_switch_{nullptr}, *h_switch_{nullptr}, *u_switch_{nullptr};
  select::Select *b_select_{nullptr};
  binary_sensor::BinarySensor *filter_alert_sensor_{nullptr},
      *link_sensor_{nullptr}, *kbd_link_sensor_{nullptr};

  uint32_t last_watchdog_{0}, last_spa_ms_{0}, last_kbd_ms_{0};
  uint32_t last_http_ms_{0}, last_http_start_ms_{0};
  uint32_t last_f_change_ms_{0}, last_h_change_ms_{0}, last_u_change_ms_{0}, last_b_change_ms_{0}, last_set_change_ms_{0};
  bool http_busy_{false}, lock_{false}, eedomus_enabled_{true};
  std::deque<EedomusRequest> eedomus_queue_;
  std::function<void(int, float, bool)> eedomus_callback_;
  
  uint8_t kbd_buf_[10], kbd_idx_{0};
  uint8_t spa_buf_[10], spa_idx_{0};
  bool last_alert_val_{false};

  // Ghosting State
  bool lp_f_{false}, lp_h_{false};
  uint32_t lc_f_{0}, lc_h_{0};

  static void uart_task_static(void *pvParameters) {
    MSPAUartComponent *component = static_cast<MSPAUartComponent *>(pvParameters);
    while (true) { component->uart_task(); vTaskDelay(pdMS_TO_TICKS(1)); }
  }

  void uart_task() {
    int quota = 64; 
    while (uart_spa_->available() && quota-- > 0) {
      uint8_t c;
      if (uart_spa_->read_byte(&c)) {
        if (xSemaphoreTakeRecursive(uart_mutex_, pdMS_TO_TICKS(10))) {
          uart_kbd_->write_byte(c); xSemaphoreGiveRecursive(uart_mutex_);
        }
        process_machine(c, true);
      }
    }
    quota = 64;
    while (uart_kbd_->available() && quota-- > 0) {
      uint8_t c; if (uart_kbd_->read_byte(&c)) process_machine(c, false);
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  void process_machine(uint8_t c, bool from_spa) {
    uint8_t &idx = from_spa ? spa_idx_ : kbd_idx_;
    uint8_t *buf = from_spa ? spa_buf_ : kbd_buf_;
    uint32_t &last_ms = from_spa ? last_spa_ms_ : last_kbd_ms_;
    uint32_t now = millis();
    if (idx > 0 && (now - last_ms > 100)) idx = 0;
    last_ms = now;
    if (idx == 0) { if (c == 0xA5 || c == 0x00) { buf[0] = c; idx = 1; } return; }
    if (idx < 9) {
      buf[idx++] = c;
      int len = (buf[0] == 0x00) ? 5 : ((buf[1] == 0x1B) ? 5 : 4);
      if (idx == len) {
        bool valid = false;
        if (buf[0] == 0xA5) { uint8_t cs = 0; for (int i = 0; i < len - 1; i++) cs += buf[i]; valid = (buf[len - 1] == cs); }
        else if (buf[0] == 0x00) { uint8_t cs = (uint8_t)(buf[1] + buf[2] + buf[3] - 0x11); valid = (buf[len - 1] == cs); }
        if (valid) {
          if (!from_spa && (!lock_ || buf[1] == 0x0D)) {
            if (xSemaphoreTakeRecursive(uart_mutex_, pdMS_TO_TICKS(10))) { uart_spa_->write_array(buf, len); xSemaphoreGiveRecursive(uart_mutex_); }
          }
          handle_frame(buf[1], buf[2], (len == 5 ? buf[3] : 0), from_spa);
        }
        idx = 0;
      }
    }
  }

  void handle_frame(uint8_t id, uint8_t d1, uint8_t d2, bool from_spa) {
    if (!from_spa) return;
    uint32_t now = millis();
    if (id == 0x08) { physical_f_on_ = (d1 & 0x01); physical_h_on_ = (d1 & 0x02); }
    if (id == 0x1A) {
      bool pf = (d1 & 0x01), ph = (d1 & 0x02), pu = (d1 & 0x04);
      auto detect_ghost = [&](bool current, bool &last_state, uint32_t &last_change, std::atomic<bool> &is_ghost) {
        if (current != last_state) {
          uint32_t dt = now - last_change;
          if (dt > 50 && dt < 1500) is_ghost = true;
          last_change = now; last_state = current;
        }
        if (now - last_change > 2000) is_ghost = false;
      };
      detect_ghost(pf, lp_f_, lc_f_, is_blinking_f_);
      detect_ghost(ph, lp_h_, lc_h_, is_blinking_h_);

      bool func_f = pf; if (is_blinking_f_.load()) func_f = physical_f_on_.load();
      bool func_h = ph; if (is_blinking_h_.load()) func_h = true;

      bus_f_ = func_f; bus_h_ = func_h; bus_u_ = pu;

      update_ui_switch(now, func_f, real_f_, last_f_change_ms_, f_switch_, is_blinking_f_.load());
      update_ui_switch(now, func_h, real_h_, last_h_change_ms_, h_switch_, is_blinking_h_.load());
      update_ui_switch(now, pu, real_u_, last_u_change_ms_, u_switch_, false);
    }
    if (id == 0x1B) {
      int pb = d1; bus_b_ = pb;
      if (pb != real_b_.load()) { if (now - last_b_change_ms_ > 1500) { real_b_ = pb; if (b_select_) b_select_->publish_state((pb == 1) ? "Niveau1" : (pb == 2) ? "Niveau2" : (pb == 3) ? "Niveau3" : "Arret"); } }
      else last_b_change_ms_ = now;
      float ps = 30.0f + (int8_t)d2;
      if (std::abs(ps - real_setpoint_.load()) > 0.1f) { if (now - last_set_change_ms_ > 1500) { real_setpoint_ = ps; ESP_LOGI(TAG, "Sync: Setpoint -> %.1f", ps); if (setpoint_sensor_) setpoint_sensor_->publish_state(ps); } }
      else last_set_change_ms_ = now;
    }
    if (id == 0x06) {
      float nt = d1 / 2.0f; if (std::abs(nt - real_temp_.load()) > 0.1f) { real_temp_ = nt; ESP_LOGI(TAG, "Sync: Water Temp -> %.1f", nt); if (temp_sensor_) temp_sensor_->publish_state(nt); }
    }
    if (id == 0x06 || id == 0x1B || id == 0x1A) {
      check_sniper(target_f_, bus_f_, retry_f_, 0x02);
      check_sniper(target_h_, bus_h_, retry_h_, 0x01);
      check_sniper(target_u_, bus_u_, retry_u_, 0x19);
      if (target_b_.load() != bus_b_.load() && retry_b_ > 0) { if (inject_cmd(0x03, (uint8_t)target_b_.load())) retry_b_--; }
    }
  }

  void update_ui_switch(uint32_t now, bool current, std::atomic<bool> &real, uint32_t &last_change, switch_::Switch *sw, bool force) {
    if (current != real.load()) {
      if (now - last_change > 1500 || force) {
        real = current; last_change = now;
        ESP_LOGI(TAG, "UI: %s -> %s (Filter: %s)", sw ? sw->get_name().c_str() : "Switch", current ? "ON" : "OFF", force ? "GHOST" : "SILK");
        if (sw) sw->publish_state(current);
      }
    } else last_change = now;
  }

  void check_sniper(std::atomic<bool> &target, std::atomic<bool> &bus, std::atomic<uint8_t> &retry, uint8_t cmd) {
    if (target.load() != bus.load() && retry > 0) { if (inject_cmd(cmd, target.load() ? 0x01 : 0x00)) retry--; }
  }
};

} // namespace mspa
} // namespace esphome
