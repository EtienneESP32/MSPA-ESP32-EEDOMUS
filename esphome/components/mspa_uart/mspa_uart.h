#pragma once
#include "esphome.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/uart/uart.h"
#include <atomic>
#include <deque>
#include <functional>

namespace esphome {
namespace mspa {

static const char *TAG = "mspa_uart";

struct EedomusRequest {
  int periph_id;
  float value;
  bool is_float;
};

class MSPAUartComponent : public esphome::Component {
public:
  MSPAUartComponent(uart::UARTComponent *uart_spa,
                    uart::UARTComponent *uart_kbd)
      : uart_spa_(uart_spa), uart_kbd_(uart_kbd) {}

  void setup() override {
    ESP_LOGI(TAG, "MSPA UART v7.1.0-STABLE (ESP-IDF) initialized");
    uart_mutex_ = xSemaphoreCreateMutex();
    kbd_idx_ = 0;
    spa_idx_ = 0;
    last_watchdog_ = millis();
    last_spa_packet_ms_ = millis();
    last_kbd_packet_ms_ = millis();

    xTaskCreatePinnedToCore(
        [](void *params) {
          static_cast<MSPAUartComponent *>(params)->uart_task();
        },
        "uart_task", 4096, this, 10, &uart_task_handle_, 1);
  }

  void set_temp_sensor(sensor::Sensor *s) { temp_sensor_ = s; }
  void set_setpoint_sensor(number::Number *s) { setpoint_sensor_ = s; }
  void set_f_switch(switch_::Switch *s) { f_switch_ = s; }
  void set_h_switch(switch_::Switch *s) { h_switch_ = s; }
  void set_u_switch(switch_::Switch *s) { u_switch_ = s; }
  void set_b_select(select::Select *s) { b_select_ = s; }
  void set_filter_alert_sensor(binary_sensor::BinarySensor *s) {
    filter_alert_sensor_ = s;
  }
  void set_link_sensor(binary_sensor::BinarySensor *s) { link_sensor_ = s; }
  void set_kbd_link_sensor(binary_sensor::BinarySensor *s) {
    kbd_link_sensor_ = s;
  }
  void set_reporter_callback(std::function<void(int, float, bool)> f) {
    reporter_callback_ = f;
  }
  void set_http_busy(bool busy) {
    http_busy_ = busy;
    if (busy)
      last_http_start_ms_ = millis();
  }

  void control_filtration(bool active) {
    target_f_ = active;
    retry_f_ = 3;
  }
  void control_heating(bool active) {
    target_h_ = active;
    retry_h_ = 3;
  }
  void control_uvc(bool active) {
    target_u_ = active;
    retry_u_ = 3;
  }
  void control_bubbles(uint8_t level) {
    target_b_ = level;
    retry_b_ = 3;
  }
  float get_current_temp() { return (float)current_temp_internal_; }
  void set_lock(bool mode) { lock_ = mode; }

  void inject_cmd(uint8_t id, uint8_t val) {
    uint8_t b[4] = {0xA5, id, val, (uint8_t)(0xA5 + id + val)};
    if (xSemaphoreTake(uart_mutex_, portMAX_DELAY)) {
      uart_spa_->write_array(b, 4);
      xSemaphoreGive(uart_mutex_);
      ESP_LOGI(TAG, "Sniper Shot: ID=0x%02X VAL=0x%02X", id, val);
    }
  }

  void enqueue_eedomus(int periph_id, float value, bool is_float = false,
                       bool priority = false) {
    if (uart_mutex_ == nullptr) return;
    if (xSemaphoreTake(uart_mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
      for (auto it = eedomus_queue_.begin(); it != eedomus_queue_.end(); ++it) {
        if (it->periph_id == periph_id) {
          it->value = value;
          it->is_float = is_float;
          if (priority) {
            EedomusRequest req = *it;
            eedomus_queue_.erase(it);
            eedomus_queue_.push_front(req);
            ESP_LOGD(TAG, "[QUEUE] ID %d promoted to FRONT", periph_id);
          }
          xSemaphoreGive(uart_mutex_);
          return;
        }
      }
      if (priority) {
        eedomus_queue_.push_front({periph_id, value, is_float});
        ESP_LOGI(TAG, "[QUEUE] Enqueue PRIORITY ID: %d (Size: %d)", periph_id, (int)eedomus_queue_.size());
      } else {
        eedomus_queue_.push_back({periph_id, value, is_float});
        ESP_LOGD(TAG, "[QUEUE] Enqueue ID: %d (Size: %d)", periph_id, (int)eedomus_queue_.size());
      }
      xSemaphoreGive(uart_mutex_);
    }
  }

  void loop() override {
    uint32_t now = millis();
    if (http_busy_ && (now - last_http_start_ms_ > 10000)) {
      ESP_LOGW(TAG, "[QUEUE] HTTP Timeout! Force releasing busy flag.");
      http_busy_ = false;
    }

    if (now - last_watchdog_ > 1500) {
      last_watchdog_ = now;
      run_watchdog();
      if (link_sensor_)
        link_sensor_->publish_state(now - last_spa_packet_ms_ < 3500);
      if (kbd_link_sensor_)
        kbd_link_sensor_->publish_state(now - last_kbd_packet_ms_ < 3500);
    }

    if (http_busy_ && !eedomus_queue_.empty()) {
        static uint32_t last_busy_log = 0;
        if (now - last_busy_log > 5000) {
            ESP_LOGW(TAG, "[QUEUE] Waiting for HTTP response... (Queue size: %d)", (int)eedomus_queue_.size());
            last_busy_log = now;
        }
    }

    if (!http_busy_ && !eedomus_queue_.empty() &&
        (now - last_http_ms_ > 3000)) {
      if (esp_get_free_heap_size() > 10240) {
        last_http_ms_ = now;
        EedomusRequest req = eedomus_queue_.front();
        eedomus_queue_.pop_front();
        
        ESP_LOGI(TAG, "[QUEUE] Processing ID: %d (Remaining: %d)", req.periph_id, (int)eedomus_queue_.size());
        if (reporter_callback_) {
          set_http_busy(true);
          reporter_callback_(req.periph_id, req.value, req.is_float);
        }
      }
    }
  }

protected:
  uart::UARTComponent *uart_spa_;
  uart::UARTComponent *uart_kbd_;
  TaskHandle_t uart_task_handle_;
  SemaphoreHandle_t uart_mutex_{nullptr};

  sensor::Sensor *temp_sensor_{nullptr};
  number::Number *setpoint_sensor_{nullptr};
  switch_::Switch *f_switch_{nullptr}, *h_switch_{nullptr}, *u_switch_{nullptr};
  select::Select *b_select_{nullptr};
  binary_sensor::BinarySensor *filter_alert_sensor_{nullptr},
      *link_sensor_{nullptr}, *kbd_link_sensor_{nullptr};

  std::atomic<bool> real_f_{false}, real_h_{false}, real_u_{false},
      lock_{false}, physical_f_on_{false};
  std::atomic<uint8_t> real_b_{0}, target_b_{0};
  std::atomic<bool> target_f_{false}, target_h_{false}, target_u_{false};
  std::atomic<uint8_t> retry_f_{0}, retry_h_{0}, retry_u_{0}, retry_b_{0};
  std::atomic<float> current_temp_internal_{0.0f};

  uint32_t last_watchdog_{0}, last_spa_packet_ms_{0}, last_kbd_packet_ms_{0};
  uint32_t last_http_ms_{0}, last_http_start_ms_{0};
  std::atomic<bool> http_busy_{false};
  std::deque<EedomusRequest> eedomus_queue_;
  std::function<void(int, float, bool)> reporter_callback_{nullptr};

  uint8_t kbd_buf_[10], kbd_idx_{0};
  uint8_t spa_buf_[10], spa_idx_{0};
  uint32_t last_on_f_{0}, last_on_h_{0}, last_on_u_{0};
  bool is_blinking_f_{false}, last_f_on_{false}, last_alert_val_{false};

  void uart_task() {
    uint8_t c;
    while (true) {
      // SPA -> KBD (Transparent Immédiat)
      while (uart_spa_->available()) {
        if (uart_spa_->read_byte(&c)) {
          if (xSemaphoreTake(uart_mutex_, portMAX_DELAY)) {
            uart_kbd_->write_byte(c);
            xSemaphoreGive(uart_mutex_);
          }
          process_machine(c, spa_buf_, spa_idx_, true);
        }
      }
      // KBD -> SPA (Filtré pour MITM/Lock)
      while (uart_kbd_->available()) {
        if (uart_kbd_->read_byte(&c)) {
          process_machine(c, kbd_buf_, kbd_idx_, false);
        }
      }
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }

  void process_machine(uint8_t c, uint8_t *buf, uint8_t &idx, bool from_spa) {
    if (c == 0xA5 || (idx == 0 && c == 0x00)) {
      buf[0] = c;
      idx = 1;
      return;
    }
    if (idx > 0 && idx < 9) {
      buf[idx++] = c;
      if (buf[0] == 0xA5) {
        int len = (buf[1] == 0x1B) ? 5 : 4;
        if (idx == len) {
          uint8_t sum = 0;
          for (int i = 0; i < len - 1; i++)
            sum += buf[i];
          if (sum == buf[len - 1]) {
            if (!from_spa) {
              if (!lock_ || buf[1] == 0x0D) {
                if (xSemaphoreTake(uart_mutex_, portMAX_DELAY)) {
                  uart_spa_->write_array(buf, len);
                  xSemaphoreGive(uart_mutex_);
                }
              }
            }
            handle_frame(buf[1], buf[2], (len == 5 ? buf[3] : 0), from_spa);
          }
          idx = 0;
        }
      } else if (buf[0] == 0x00 && idx == 5) {
        uint8_t sum = (uint8_t)(buf[1] + buf[2] + buf[3] - 0x11);
        if (sum == buf[4]) {
          if (!from_spa && !lock_) {
            if (xSemaphoreTake(uart_mutex_, portMAX_DELAY)) {
              uart_spa_->write_array(buf, 5);
              xSemaphoreGive(uart_mutex_);
            }
          }
          handle_frame(buf[1], buf[2], buf[3], from_spa);
        }
        idx = 0;
      }
    }
    if (idx >= 10)
      idx = 0;
    if (from_spa)
      last_spa_packet_ms_ = millis();
    else
      last_kbd_packet_ms_ = millis();
  }

  void handle_frame(uint8_t id, uint8_t d1, uint8_t d2, bool from_spa) {
    if (!from_spa)
      return;
    uint32_t now = millis();
    if (id == 0x1A) {
      bool f_on = (d1 & 0x01);
      if (f_on) {
        if (!last_f_on_ && (now - last_on_f_ > 400 && now - last_on_f_ < 1500))
          is_blinking_f_ = true;
        else if (!last_f_on_)
          is_blinking_f_ = false;
        last_on_f_ = now;
      } else if (now - last_on_f_ > 1500)
        is_blinking_f_ = false;
      last_f_on_ = f_on;
      if (d1 & 0x02)
        last_on_h_ = now;
      if (d1 & 0x04)
        last_on_u_ = now;
    } else if (id == 0x08) {
      physical_f_on_ = (d1 & 0x01);
    } else if (id == 0x1B) {
      if (d1 != real_b_) {
        real_b_ = d1;
        target_b_ = d1;
        retry_b_ = 0;
        if (b_select_)
          b_select_->publish_state(d1 == 1   ? "Niveau1"
                                   : d1 == 2 ? "Niveau2"
                                   : d1 == 3 ? "Niveau3"
                                             : "Arret");
      }
      if (setpoint_sensor_) {
        float val = d2 + 30.0f;
        if (val >= 20.0f && val <= 42.0f && setpoint_sensor_->state != val)
          setpoint_sensor_->publish_state(val);
      }
    } else if (id == 0x06) {
      current_temp_internal_ = d1 / 2.0f;
      if (temp_sensor_)
        temp_sensor_->publish_state(current_temp_internal_);
    }
  }

  void run_watchdog() {
    uint32_t now = millis();
    update_silk(now, last_on_f_, real_f_, target_f_, retry_f_, f_switch_,
                physical_f_on_);
    update_silk(now, last_on_h_, real_h_, target_h_, retry_h_, h_switch_);
    update_silk(now, last_on_u_, real_u_, target_u_, retry_u_, u_switch_);
    if (target_b_ != real_b_ && retry_b_ > 0) {
      retry_b_--;
      inject_cmd(0x03, target_b_);
    }
    if (filter_alert_sensor_) {
      bool alert = (is_blinking_f_ && !physical_f_on_);
      if (alert != last_alert_val_) {
        last_alert_val_ = alert;
        filter_alert_sensor_->publish_state(alert);
      }
    }
  }

  void update_silk(uint32_t now, uint32_t last_on, std::atomic<bool> &real,
                   std::atomic<bool> &target, std::atomic<uint8_t> &retry,
                   switch_::Switch *sw, bool phys = true) {
    bool active = (last_on > 0 && (now - last_on < 1500)) && phys;
    if (active != real) {
      real = active;
      target = (bool)real;
      retry = 0;
      if (sw)
        sw->publish_state(real);
    }
    if (target != real && retry > 0) {
      retry--;
      inject_cmd((sw == h_switch_)   ? 0x01
                 : (sw == f_switch_) ? 0x02
                                     : 0x19,
                 target ? 0x01 : 0x00);
    }
  }
};

} // namespace mspa
} // namespace esphome
