#pragma once
#include "esphome.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace mspa {

static const char *TAG = "mspa_uart";

class MSPAUartComponent : public esphome::Component {
public:
  MSPAUartComponent(uart::UARTComponent *uart_spa,
                    uart::UARTComponent *uart_kbd)
      : uart_spa_(uart_spa), uart_kbd_(uart_kbd) {}

  void setup() override {
    ESP_LOGI(TAG, "MSPA UART v6.3.2 'State Sync' initialized");
    kbd_idx_ = 0;
    spa_idx_ = 0;
    memset(kbd_buf_, 0, 10);
    memset(spa_buf_, 0, 10);
    last_watchdog_ = millis();
    last_on_f_ = last_on_h_ = last_on_u_ = last_on_b_ = 0;
    real_f_ = real_h_ = real_u_ = false;
    blink_start_f_ = 0;
    is_blinking_f_ = false;
  }

  // --- CONFIGURATION ---
  void set_temp_sensor(sensor::Sensor *s) { temp_sensor_ = s; }
  void set_setpoint_sensor(number::Number *s) { setpoint_sensor_ = s; }
  void set_f_switch(switch_::Switch *s) { f_switch_ = s; }
  void set_h_switch(switch_::Switch *s) { h_switch_ = s; }
  void set_u_switch(switch_::Switch *s) { u_switch_ = s; }
  void set_b_select(select::Select *s) { b_select_ = s; }
  void set_filter_alert_sensor(binary_sensor::BinarySensor *s) {
    filter_alert_sensor_ = s;
  }

  // --- ACTIONS PAR CIBLES ---
  void control_filtration(bool active) {
    target_f_ = active;
    retry_f_ = 3;
    inject_cmd(0x02, active ? 0x01 : 0x00);
  }
  void control_heating(bool active) {
    target_h_ = active;
    retry_h_ = 3;
    inject_cmd(0x01, active ? 0x01 : 0x00);
  }
  void control_uvc(bool active) {
    target_u_ = active;
    retry_u_ = 3;
    inject_cmd(0x19, active ? 0x01 : 0x00);
  }
  void control_bubbles(uint8_t level) {
    target_b_ = level;
    retry_b_ = 3;
    inject_cmd(0x03, level);
  }
  void set_lock(bool mode) { lock_ = mode; }

  void loop() override {
    while (uart_kbd_->available()) {
      uint8_t c;
      if (uart_kbd_->read_byte(&c)) {
        process_machine(c, kbd_buf_, kbd_idx_, false);
      }
    }
    while (uart_spa_->available()) {
      uint8_t c;
      if (uart_spa_->read_byte(&c)) {
        uart_kbd_->write_byte(c);
        process_machine(c, spa_buf_, spa_idx_, true);
      }
    }

    uint32_t now = millis();
    // MIROIR STABLE (Silk Filter)
    // MIROIR STABLE (Silk Filter) - Hybride (Icone 1A + Relais 08)
    bool f_active = (last_on_f_ > 0 && (now - last_on_f_ < 1500)) && physical_f_on_;
    if (f_active != real_f_) {
      real_f_ = f_active;
      target_f_ = real_f_;
      retry_f_ = 0;
      if (f_switch_)
        f_switch_->publish_state(real_f_);
    }
    update_silk(now, last_on_h_, real_h_, target_h_, retry_h_, h_switch_);
    update_silk(now, last_on_u_, real_u_, target_u_, retry_u_, u_switch_);

    // DETECTION ALERTE FILTRE (Déclenche si clignote ET pompe physique OFF)
    if (filter_alert_sensor_) {
      bool alert = (is_blinking_f_ && !physical_f_on_);
      if (alert != last_alert_val_) {
        last_alert_val_ = alert;
        filter_alert_sensor_->publish_state(alert);
      }
    }

    if (now - last_watchdog_ > 2500) {
      last_watchdog_ = now;
      run_watchdog();
    }
  }

protected:
  uart::UARTComponent *uart_spa_;
  uart::UARTComponent *uart_kbd_;

  sensor::Sensor *temp_sensor_{nullptr};
  number::Number *setpoint_sensor_{nullptr};
  switch_::Switch *f_switch_{nullptr}, *h_switch_{nullptr}, *u_switch_{nullptr};
  select::Select *b_select_{nullptr};
  binary_sensor::BinarySensor *filter_alert_sensor_{nullptr};

  bool real_f_{false}, real_h_{false}, real_u_{false}, physical_f_on_{false};
  bool lock_{false};
  uint8_t real_b_{0};

  bool target_f_{false}, target_h_{false}, target_u_{false};
  uint8_t target_b_{0};
  uint8_t retry_f_{0}, retry_h_{0}, retry_u_{0}, retry_b_{0};

  uint32_t last_watchdog_{0};
  uint32_t last_on_f_{0}, last_on_h_{0}, last_on_u_{0}, last_on_b_{0};

  // Alerte Filtre
  uint32_t last_handle_1A_{0};
  uint32_t blink_start_f_{0};
  bool is_blinking_f_{false};
  bool last_f_on_{false};
  bool last_alert_val_{false};

  uint8_t kbd_buf_[10], kbd_idx_{0};
  uint8_t spa_buf_[10], spa_idx_{0};

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
              // FIREWALL: Bloque si locké, sauf le 0D (Ping)
              if (!lock_ || buf[1] == 0x0D) {
                uart_spa_->write_array(buf, len);
              }
            }
            handle_frame(buf[1], buf[2], (len == 5 ? buf[3] : 0), true,
                         from_spa);
          }
          idx = 0;
        }
      } else if (buf[0] == 0x00 && idx == 5) {
        uint8_t sum = (uint8_t)(buf[1] + buf[2] + buf[3] - 0x11);
        if (sum == buf[4]) {
          if (!from_spa && !lock_) {
            uart_spa_->write_array(buf, 5);
          }
          handle_frame(buf[1], buf[2], buf[3], false, from_spa);
        }
        idx = 0;
      }
    }
    if (idx >= 10)
      idx = 0;
  }

  void handle_frame(uint8_t id, uint8_t d1, uint8_t d2, bool is_a5,
                    bool from_spa) {
    if (from_spa && is_a5) {
      uint32_t now = millis();
      if (id == 0x1A) {
        bool f_on = (d1 & 0x01);
        if (f_on) {
          if (!last_f_on_) { // Transition 0 -> 1 (Edge detection)
            uint32_t dt = now - last_on_f_;
            if (dt > 400 && dt < 1500) {
              if (!is_blinking_f_) {
                is_blinking_f_ = true;
                blink_start_f_ = now;
              }
            } else {
              is_blinking_f_ = false;
              blink_start_f_ = 0;
            }
          }
          last_on_f_ = now;
        } else {
          if (now - last_on_f_ > 1500) {
            is_blinking_f_ = false;
            blink_start_f_ = 0;
          }
        }
        last_f_on_ = f_on;
        if (d1 & 0x02)
          last_on_h_ = now;
        if (d1 & 0x04)
          last_on_u_ = now;
        last_handle_1A_ = now;
      } else if (id == 0x08) {
        physical_f_on_ = (d1 & 0x01);
      } else if (id == 0x1B) {
        // ID 1B : Retour Consigne et Bulles
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
        // Feedback Consigne (Strict Offset 30)
        if (setpoint_sensor_) {
          float val = (int8_t)d2 + 30.0f;
          if (val >= 20.0f && val <= 42.0f) {
            if (setpoint_sensor_->state != val)
              setpoint_sensor_->publish_state(val);
          } else {
            ESP_LOGW(TAG, "Consigne Bus Incohérente ignorée: %0.1f (D2=%d)",
                     val, d2);
          }
        }
      } else if (id == 0x06 && temp_sensor_) {
        temp_sensor_->publish_state(d1 / 2.0f);
      }
    }
  }

  void update_silk(uint32_t now, uint32_t last_on, bool &real, bool &target,
                   uint8_t &retry, switch_::Switch *sw) {
    bool active = (last_on > 0 && (now - last_on < 1500));
    if (active != real) {
      real = active;
      target = real;
      retry = 0;
      if (sw)
        sw->publish_state(real);
    }
  }

  void run_watchdog() {
    check_target(target_f_, real_f_, retry_f_, 0x02, f_switch_);
    check_target(target_h_, real_h_, retry_h_, 0x01, h_switch_);
    check_target(target_u_, real_u_, retry_u_, 0x19, u_switch_);
    if (target_b_ != real_b_ && retry_b_ > 0) {
      retry_b_--;
      inject_cmd(0x03, target_b_);
    } else if (retry_b_ == 0) {
      if (target_b_ != real_b_ && b_select_)
        b_select_->publish_state(real_b_ == 1   ? "Niveau1"
                                 : real_b_ == 2 ? "Niveau2"
                                 : real_b_ == 3 ? "Niveau3"
                                                : "Arret");
      target_b_ = real_b_;
    }
  }

  void check_target(bool target, bool real, uint8_t &retry, uint8_t cmd_id,
                    switch_::Switch *sw) {
    if (target != real && retry > 0) {
      retry--;
      inject_cmd(cmd_id, target ? 0x01 : 0x00);
      ESP_LOGW(TAG, "Sniper 0x%02X Retry (%d left)", cmd_id, retry);
    } else if (retry == 0) {
      if (target != real && sw)
        sw->publish_state(real);
      target = real;
    }
  }

  void inject_cmd(uint8_t id, uint8_t val) {
    uint8_t b[4] = {0xA5, id, val, (uint8_t)(0xA5 + id + val)};
    uart_spa_->write_array(b, 4);
    ESP_LOGI(TAG, "UI Pulse Sent: ID=0x%02X VAL=0x%02X", id, val);
  }
};

} // namespace mspa
} // namespace esphome
