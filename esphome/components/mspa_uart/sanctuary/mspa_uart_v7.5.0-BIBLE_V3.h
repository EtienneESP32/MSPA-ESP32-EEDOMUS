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

  std::atomic<bool> target_f_{false}, target_h_{false}, target_u_{false};
  std::atomic<int> target_b_{0}, real_b_{0};
  std::atomic<bool> real_f_{false}, real_h_{false}, real_u_{false};
  std::atomic<uint8_t> retry_f_{0}, retry_h_{0}, retry_u_{0}, retry_b_{0};
  std::atomic<bool> physical_f_on_{false}, physical_h_on_{false};
  std::atomic<bool> is_blinking_f_{false}, last_f_on_{false},
      last_alert_val_{false};
  std::atomic<bool> bus_f_{false}, bus_h_{false}, bus_u_{false};
  std::atomic<int> bus_b_{0};
  std::atomic<uint32_t> last_on_f_{0}, last_injection_ms_{0};

  void enqueue_eedomus(int p_id, float val, bool is_f, bool force = false) {
    if (eedomus_queue_.size() > 20)
      eedomus_queue_.pop_back();
    if (force) {
      eedomus_queue_.push_front({p_id, val, is_f});
    } else {
      eedomus_queue_.push_back({p_id, val, is_f});
    }
  }

  void setup() override {
    ESP_LOGI(TAG, "MSPA v7.5.0-FINAL-B3 Starting...");
    uart_mutex_ = xSemaphoreCreateMutex();
    if (uart_mutex_ != NULL) {
      xTaskCreatePinnedToCore(MSPAUartComponent::uart_task_static,
                              "mspa_uart_task", 4096, this, 5,
                              &uart_task_handle_, 1);
    }
  }

  void dump_config() override {
    ESP_LOGCONFIG(TAG, "MSPA UART Component:");
    ESP_LOGCONFIG(TAG, "  HTTP Busy: %s", http_busy_ ? "YES" : "NO");
    ESP_LOGCONFIG(TAG, "  Queue Size: %d", eedomus_queue_.size());
  }

  void loop() override {
    static uint32_t last_loop_log = 0;
    uint32_t now = millis();
    if (now - last_loop_log > 10000) {
      ESP_LOGD(TAG, "Component Loop Heartbeat (Queue: %d)",
               eedomus_queue_.size());
      last_loop_log = now;
    }
    if (http_busy_ && (now - last_http_start_ms_ > 30000))
      http_busy_ = false;
    if (!http_busy_ && !eedomus_queue_.empty() &&
        (now - last_http_ms_ > 5000)) {
      EedomusRequest req = eedomus_queue_.front();
      eedomus_queue_.pop_front();
      last_http_ms_ = now;
      last_http_start_ms_ = now;
      if (eedomus_callback_) {
        set_http_busy(true);
        eedomus_callback_(req.periph_id, req.value, req.is_float);
      }
    }
    if (now - last_watchdog_ > 1500) {
      last_watchdog_ = now;
      run_watchdog();
    }
  }

  void control_filtration(bool state) {
    ESP_LOGI(TAG, "UI: Control Filtration -> %s", state ? "ON" : "OFF");
    if (target_f_.load() != state) {
      target_f_ = state;
      retry_f_ = 5;
    }
  }
  void control_heating(bool state) {
    ESP_LOGI(TAG, "UI: Control Heating -> %s", state ? "ON" : "OFF");
    if (target_h_.load() != state) {
      target_h_ = state;
      retry_h_ = 5;
    }
  }
  void control_uvc(bool state) {
    ESP_LOGI(TAG, "UI: Control UVC -> %s", state ? "ON" : "OFF");
    if (target_u_.load() != state) {
      target_u_ = state;
      retry_u_ = 5;
    }
  }

  void control_bubbles(int level) {
    if (target_b_.load() != level) {
      target_b_ = level;
      retry_b_ = 5;
      last_pump_start_ms_ = millis();
      inject_cmd(0x03, (uint8_t)level); // Immediate injection
    }
  }

  float get_current_temp() { return real_temp_; }
  bool get_display_f() {
    return (millis() - last_pump_start_ms_.load() < 30000) ? target_f_.load()
                                                           : real_f_.load();
  }
  bool get_display_h() {
    return (millis() - last_pump_start_ms_.load() < 30000) ? target_h_.load()
                                                           : real_h_.load();
  }
  bool get_display_u() {
    return (millis() - last_pump_start_ms_.load() < 30000) ? target_u_.load()
                                                           : real_u_.load();
  }
  int get_display_b() {
    return real_b_.load();
  }

  bool inject_cmd(uint8_t id, uint8_t val) {
    uint8_t buf[5] = {0xA5, id, val, 0x00, 0x00};
    int len = 4; // Bible V3: All commands to SPA are 4 bytes
    buf[3] = (uint8_t)(0xA5 + id + val);

    uint32_t now = millis();
    if (now - last_injection_ms_.load() < 200) {
      return false; // Throttle to 200ms between shots
    }

    if (xSemaphoreTake(uart_mutex_, pdMS_TO_TICKS(100))) {
      ESP_LOGI(TAG, "Injecting CMD: ID=0x%02X VAL=0x%02X (Len=%d)", id, val,
               len);
      uart_spa_->write_array(buf, len);
      last_injection_ms_ = now;
      xSemaphoreGive(uart_mutex_);
      return true;
    }
    return false;
  }

protected:
  static void uart_task_static(void *params) {
    ((MSPAUartComponent *)params)->uart_task();
  }

  void uart_task() {
    while (true) {
      bool activity = false;
      // SPA -> KBD (Transparent Immédiat)
      while (uart_spa_->available()) {
        uint8_t c;
        if (uart_spa_->read_byte(&c)) {
          if (xSemaphoreTake(uart_mutex_, portMAX_DELAY)) {
            uart_kbd_->write_byte(c);
            xSemaphoreGive(uart_mutex_);
          }
          process_machine(c, true);
          activity = true;
        }
      }
      // KBD -> SPA (Filtré dans process_machine)
      while (uart_kbd_->available()) {
        uint8_t c;
        if (uart_kbd_->read_byte(&c)) {
          process_machine(c, false);
          activity = true;
        }
      }
      vTaskDelay(pdMS_TO_TICKS(activity ? 1 : 10));
    }
  }

  void process_machine(uint8_t c, bool from_spa) {
    uint8_t &idx = from_spa ? spa_idx_ : kbd_idx_;
    uint8_t *buf = from_spa ? spa_buf_ : kbd_buf_;
    uint32_t &last_ms = from_spa ? last_spa_ms_ : last_kbd_ms_;
    uint32_t now = millis();

    if (idx > 0 && (now - last_ms > 100))
      idx = 0; // Robust timeout
    last_ms = now;

    // 1. SYNC: Only if we are at the start
    if (idx == 0) {
      if (c == 0xA5 || c == 0x00) {
        buf[0] = c;
        idx = 1;
      }
      return;
    }

    // 2. COLLECT: Fill the buffer
    if (idx < 9) {
      buf[idx++] = c;

      // 3. IDENTIFY FRAME TYPE & LENGTH
      int len = 0;
      bool is_extended = (buf[0] == 0x00);
      if (!is_extended) {
        len = (buf[1] == 0x1B) ? 5 : 4;
      } else {
        len = 5; // Extended frames are always 5 bytes
      }

      // 4. PROCESS COMPLETE FRAME
      if (idx == len) {
        uint8_t cs = 0;
        bool valid = false;

        if (!is_extended) {
          // Standard Checksum: Sum
          for (int i = 0; i < len - 1; i++)
            cs += buf[i];
          valid = (buf[len - 1] == cs);
        } else {
          // Extended Checksum: Sum - 17 (0x11)
          cs = (uint8_t)(buf[1] + buf[2] + buf[3] - 0x11);
          valid = (buf[len - 1] == cs);
        }

        if (valid) {
          if (!from_spa) {
            uint8_t id = buf[1];
            uint8_t d1 = buf[2];

            // FIREWALL & SYNC TARGETS
            if (id == 0x01) { // HEAT
              if (lock_) {
                buf[2] = real_h_.load() ? 0x01 : 0x00;
                buf[len - 1] = is_extended ? (uint8_t)(buf[1] + buf[2] + buf[3] - 0x11) : (uint8_t)(buf[0] + buf[1] + buf[2]);
              } else {
                target_h_ = (d1 > 0);
                retry_h_ = 5; // ARM SNIPER
              }
            } else if (id == 0x02) { // FILT
              if (lock_) {
                buf[2] = real_f_.load() ? 0x01 : 0x00;
                buf[len - 1] = is_extended ? (uint8_t)(buf[1] + buf[2] + buf[3] - 0x11) : (uint8_t)(buf[0] + buf[1] + buf[2]);
              } else {
                target_f_ = (d1 > 0);
                retry_f_ = 5; // ARM SNIPER
              }
            } else if (id == 0x19) { // UVC
              if (lock_) {
                buf[2] = real_u_.load() ? 0x01 : 0x00;
                buf[len - 1] = is_extended ? (uint8_t)(buf[1] + buf[2] + buf[3] - 0x11) : (uint8_t)(buf[0] + buf[1] + buf[2]);
              } else {
                target_u_ = (d1 > 0);
                retry_u_ = 5; // ARM SNIPER
              }
            } else if (id == 0x03 || (is_extended && id == 0x11)) { // BUBBLES
              target_b_ = d1;
              retry_b_ = 5; // ARM SNIPER
              ESP_LOGI(TAG, "KBD: Bubble Command Detected (ID=0x%02X Lvl=%d)", id, d1);
            } else if (id == 0x04) { // SETPOINT
              // Passive capture of manual setpoint changes
            }

            // FORWARD TO SPA
            if (xSemaphoreTake(uart_mutex_, portMAX_DELAY)) {
              uart_spa_->write_array(buf, len);
              xSemaphoreGive(uart_mutex_);
            }
          } else {
            // FORWARD TO KBD (Byte by byte is already done in uart_task, but here we handle logic)
          }

          // COMMON FRAME HANDLING
          handle_frame(buf[1], buf[2], (len == 5 ? buf[3] : 0), from_spa);
        }
        idx = 0; // Reset for next frame
      }
    }
    if (idx >= 10)
      idx = 0;
  }

  void handle_frame(uint8_t id, uint8_t d1, uint8_t d2, bool from_spa) {
    ESP_LOGD(TAG, "Frame from %s: ID=0x%02X D1=0x%02X D2=0x%02X",
             from_spa ? "SPA" : "KBD", id, d1, d2);
    uint32_t now = millis();
    if (from_spa) {
      last_spa_activity_ = now;
      if (id == 0x08) {
        ESP_LOGD(TAG, "Status 0x08 Bits: D1=%02X", d1);
        physical_f_on_ = (d1 & 0x01);
        physical_h_on_ = (d1 & 0x02);
      }
      if (id == 0x1A) {
        ESP_LOGD(TAG, "Status 0x1A Bits: D1=%02X", d1);
        bool pf = (d1 & 0x01), ph = (d1 & 0x02), pu = (d1 & 0x04);

        // Blinking Detection (v6.9 Logic)
        if (pf) {
          if (!last_f_on_ &&
              (now - last_on_f_ > 400 && now - last_on_f_ < 1500)) {
            is_blinking_f_ = true;
            ESP_LOGW(TAG, "Filter Icon BLINKING detected!");
          } else if (!last_f_on_)
            is_blinking_f_ = false;
          last_on_f_ = now;
        } else if (now - last_on_f_ > 1500) {
          if (is_blinking_f_)
            ESP_LOGD(TAG, "Filter Icon stopped blinking.");
          is_blinking_f_ = false;
        }
        last_f_on_ = pf;

        if (pf != bus_f_.load()) {
          bus_f_ = pf;
        }
        // SILK FILTER: 1500ms stability based on Bible v3
        if (pf != real_f_.load()) {
          if (now - last_f_change_ms_ > 1500) {
            real_f_ = pf;
            if (retry_f_ == 0)
              target_f_ = pf;
            if (f_switch_)
              f_switch_->publish_state(pf);
          }
        } else {
          last_f_change_ms_ = now;
        }
        if (ph != bus_h_.load()) {
          bus_h_ = ph;
        }
        if (ph != real_h_.load()) {
          if (now - last_h_change_ms_ > 1500) {
            real_h_ = ph;
            if (retry_h_ == 0)
              target_h_ = ph;
            if (h_switch_)
              h_switch_->publish_state(ph);
          }
        } else {
          last_h_change_ms_ = now;
        }
        if (pu != bus_u_.load()) {
          bus_u_ = pu;
        }
        if (pu != real_u_.load()) {
          if (now - last_u_change_ms_ > 1500) {
            real_u_ = pu;
            if (retry_u_ == 0)
              target_u_ = pu;
            if (u_switch_)
              u_switch_->publish_state(pu);
          }
        } else {
          last_u_change_ms_ = now;
        }
      }
      if (id == 0x1B) {
        int pb = d1;
        if (pb != bus_b_.load()) {
          bus_b_ = pb;
        }
        if (pb != real_b_.load()) {
          if (now - last_b_change_ms_ > 1500) {
            real_b_ = pb;
            if (retry_b_ == 0)
              target_b_ = pb;
            if (b_select_) {
              b_select_->publish_state((pb == 1)   ? "Niveau1"
                                       : (pb == 2) ? "Niveau2"
                                       : (pb == 3) ? "Niveau3"
                                                   : "Arret");
            }
          }
        } else {
          last_b_change_ms_ = now;
        }
        float ps = 30.0f + (int8_t)d2;
        if (std::abs(ps - real_setpoint_) > 0.1f) {
          if (now - last_set_change_ms_ > 1500) {
            real_setpoint_ = ps;
            if (setpoint_sensor_)
              setpoint_sensor_->publish_state(ps);
          }
        } else {
          last_set_change_ms_ = now;
        }
      }
      if (id == 0x06) {
        float new_temp = d1 / 2.0f;
        if (std::abs(new_temp - real_temp_) > 0.1f) {
          real_temp_ = new_temp;
          ESP_LOGI(TAG, "Silk Sync: Water Temp is %.1f", new_temp);
        }
      }

      // Sniper Injection (Parallel) - Using BUS state for immediate feedback
      if (id == 0x06 || id == 0x1B) {
        if (target_f_.load() != bus_f_.load() && retry_f_ > 0) {
          if (inject_cmd(0x02, target_f_.load() ? 0x01 : 0x00))
            retry_f_--;
        }
        if (target_h_.load() != bus_h_.load() && retry_h_ > 0) {
          if (inject_cmd(0x01, target_h_.load() ? 0x01 : 0x00))
            retry_h_--;
        }
        if (target_u_.load() != bus_u_.load() && retry_u_ > 0) {
          if (inject_cmd(0x19, target_u_.load() ? 0x01 : 0x00))
            retry_u_--;
        }
        if (target_b_.load() != bus_b_.load() && retry_b_ > 0) {
          if (inject_cmd(0x03, (uint8_t)target_b_.load()))
            retry_b_--;
        }
      }
    } else {
      last_kbd_activity_ = now;
    }
  }

public:
  void run_watchdog() {
    uint32_t now = millis();
    if (link_sensor_)
      link_sensor_->publish_state(now - last_spa_activity_ < 5000);
    if (kbd_link_sensor_)
      kbd_link_sensor_->publish_state(now - last_kbd_activity_ < 5000);
    if (temp_sensor_ && real_temp_ > 0)
      temp_sensor_->publish_state(real_temp_);

    // Filter Alert Logic (Blinking detection)
    if (filter_alert_sensor_) {
      bool alert = (is_blinking_f_ && !physical_f_on_.load());
      if (alert)
        ESP_LOGW(TAG, "ALERT FILTER ACTIVE!");
      if (alert != last_alert_val_) {
        last_alert_val_ = alert;
        filter_alert_sensor_->publish_state(alert);
      }
    }
  }

  uart::UARTComponent *uart_spa_{nullptr}, *uart_kbd_{nullptr};
  sensor::Sensor *temp_sensor_{nullptr};
  number::Number *setpoint_sensor_{nullptr};
  binary_sensor::BinarySensor *link_sensor_{nullptr},
      *kbd_link_sensor_{nullptr}, *filter_alert_sensor_{nullptr};
  switch_::Switch *f_switch_{nullptr}, *h_switch_{nullptr}, *u_switch_{nullptr};
  select::Select *b_select_{nullptr};

  uint8_t spa_buf_[10], spa_idx_{0}, kbd_buf_[10], kbd_idx_{0};
  uint32_t last_spa_ms_{0}, last_kbd_ms_{0}, last_watchdog_{0},
      last_http_ms_{0}, last_http_start_ms_{0};
  uint32_t last_spa_activity_{0}, last_kbd_activity_{0};
  float real_temp_{0}, real_setpoint_{0};
  std::atomic<uint32_t> last_pump_start_ms_{0};
  std::atomic<bool> http_busy_{false}, lock_{false};
  std::deque<EedomusRequest> eedomus_queue_;
  uint8_t last_status_1a_{0xFF};
  std::function<void(int, float, bool)> eedomus_callback_;
  SemaphoreHandle_t uart_mutex_;
  TaskHandle_t uart_task_handle_;
  uint32_t last_f_change_ms_{0}, last_h_change_ms_{0}, last_u_change_ms_{0},
      last_b_change_ms_{0}, last_set_change_ms_{0};
  uint8_t last_kbd_f_{0}, last_kbd_h_{0}, last_kbd_u_{0};
};

} // namespace mspa
} // namespace esphome
