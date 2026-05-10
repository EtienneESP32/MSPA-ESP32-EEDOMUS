#include "esphome.h"

class MspaSimTask : public Component {
 public:
  void setup() override {
    xTaskCreatePinnedToCore(
      [](void* pvParameters) {
        while (true) {
          uint32_t now = millis();
          uint8_t cycle = id(sim_cycle);
          cycle++;
          if (cycle >= 10) cycle = 0;
          id(sim_cycle) = cycle;

          bool filt = id(sim_filt_active);
          int heat_ph = id(sim_heat_phase);
          int uvc_ph = id(sim_uvc_phase);
          bool alert = id(sim_filter_alert);
          int bubbles = id(sim_bubbles);
          int setpoint = id(sim_setpoint);
          uint8_t relay = id(sim_relay);
          float temp = id(sim_temp);

          // 1. Construction du Registre Status 0x1A (AVEC GHOST PATTERN)
          uint8_t f1A = 0x08; 
          bool blink_bit = (now % 1000 < 500);

          if (alert) {
            if (blink_bit) f1A |= 0x01;
          } else {
            bool p_on = (filt || heat_ph > 0 || uvc_ph > 0);
            if (p_on) { 
              f1A |= 0x01; 
              id(sim_relay) = 0x03; 
            } else {
              // Gestion auto-pump si plus rien n'est actif (déjà fait par script pump_cooldown en YAML)
            }

            if (heat_ph == 2) {
              // MODE GHOST : Multiplexage à 5Hz (alternance par cycle de 200ms)
              if (cycle % 2 == 0) {
                f1A |= 0x01; // Filtre ON
                f1A &= ~0x02; // Chauffe OFF
              } else {
                f1A &= ~0x01; // Filtre OFF
                f1A |= 0x02; // Chauffe ON
              }
            } else if (heat_ph == 1) {
               if (cycle % 10 < 5) f1A |= 0x02;
            }

            if (uvc_ph == 1) { if (cycle % 10 < 5) f1A |= 0x04; } 
            else if (uvc_ph == 2) f1A |= 0x04;
          }
          if (f1A != 0x08) f1A &= ~0x08;

          // 2. ENVOI CYCLIQUE (Simulation du Bus à 10Hz)
          if (cycle == 2 || cycle == 7) {
             uint8_t tr[4] = {0xA5, 0x1A, f1A, (uint8_t)(0xA5+0x1A+f1A)};
             id(uart_mb).write_array(tr, 4);
             if (cycle == 2) ESP_LOGD("SIM", "Bus TX: 0x1A (f1A=0x%02X)", f1A);
          }
          if (cycle == 0) {
             uint8_t r = id(sim_relay);
             uint8_t tr[4] = {0xA5, 0x08, r, (uint8_t)(0xA5+0x08+r)}; 
             id(uart_mb).write_array(tr, 4);
          }
          if (cycle == 4) {
             uint8_t f06 = (uint8_t)(temp * 2.0);
             uint8_t tr[4] = {0xA5, 0x06, f06, (uint8_t)(0xA5+0x06+f06)}; 
             id(uart_mb).write_array(tr, 4);
          }
          if (cycle == 6) {
             uint8_t s_val = (uint8_t)(setpoint - 30);
             uint8_t tr[5] = {0xA5, 0x1B, (uint8_t)bubbles, s_val, (uint8_t)(0xA5+0x1B+bubbles+s_val)};
             id(uart_mb).write_array(tr, 5);
          }
          if (cycle == 9) {
             uint8_t tr[4] = {0xA5, 0x12, 0x01, 0xB8}; id(uart_mb).write_array(tr, 4);
          }
          if (cycle == 5) {
            uint8_t f0D[4] = {0xA5, 0x0D, 0x00, 0xB2}; id(uart_kbd).write_array(f0D, 4);
          }

          // 3. MISE À JOUR UI (Miroirs stables)
          id(sw_icon_heat).publish_state(heat_ph > 0); 
          id(sw_icon_filt).publish_state(filt || heat_ph > 0 || uvc_ph > 0);
          id(sw_icon_uvc).publish_state(uvc_ph > 0);
          id(sw_rel_pump).publish_state(id(sim_relay) == 0x03);

          vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz
        }
      },
      "sim_uart_task",
      4096,
      nullptr,
      1,
      nullptr,
      0
    );
  }
};
