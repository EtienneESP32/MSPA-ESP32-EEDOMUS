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

          // 1. Construction du Registre Status 0x1A (Réaliste natif)
          uint8_t f1A = 0x08; 
          
          bool is_heating_blink = ((now % 4000) < 2000); // Chauffe: 2s ON, 2s OFF
          bool is_polling_blink = ((now % 9000) < 1000); // Veille: 1s ON, 8s OFF
          bool blink_alert = (now % 1000 < 500);         // Alerte: Rapide 0.5s

          if (alert) {
            if (blink_alert) f1A |= 0x01;
          } else {
            if (heat_ph > 0) {
                // Chauffe active : toute la trame clignote 2s/2s (sauf le bit 0x08)
                if (is_heating_blink) {
                    f1A |= 0x01; // Pump
                    f1A |= 0x02; // Heat
                    if (uvc_ph > 0) f1A |= 0x04; // UVC
                }
                id(sim_relay) = 0x03; // Relais physiques restent ON
            } else if (filt || uvc_ph > 0) {
                // Filtration ou UVC pur (sans chauffe) : Historiquement stable
                f1A |= 0x01; // Pump
                if (uvc_ph > 0) f1A |= 0x04; // UVC
                id(sim_relay) = 0x03; // Relais physiques ON
            } else {
                // Veille / Polling : Aucun bouton actif. La carte mère s'auto-vérifie.
                // Clignotement erratique natif (on simule : 1s ON / 8s OFF)
                if (is_polling_blink) {
                    f1A |= 0x01; // Pump
                    // On ne simule pas de chauffe ni d'UVC pendant le polling
                }
                id(sim_relay) = 0x00; // Les relais physiques sont OFF en veille
            }
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

          // 3. MISE À JOUR UI (Authentic Blinking)
          bool ui_filt = (f1A & 0x01);
          bool ui_heat = (f1A & 0x02);
          bool ui_uvc = (f1A & 0x04);

          id(sw_icon_filt).publish_state(ui_filt);
          id(sw_icon_heat).publish_state(ui_heat);
          id(sw_icon_uvc).publish_state(ui_uvc);
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
