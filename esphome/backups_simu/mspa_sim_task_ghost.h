#include "esphome.h"

class MspaSimTask : public Component {
 public:
  void setup() override {
    xTaskCreatePinnedToCore(
      [](void* pvParameters) {
        while (true) {
          delay(100); 
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

          // Construction de la trame 0x1A (Status)
          uint8_t f1A_val = 0x08; 
          
          if (heat_ph == 2) {
             // MODE GHOST : On simule le multiplexage visuel
             // On alterne entre FILTRE SEUL et CHAUFFE SEULE à 10Hz
             if (cycle % 2 == 0) {
                f1A_val |= 0x01; // Filtre ON
                f1A_val &= ~0x02; // Chauffe OFF
             } else {
                f1A_val &= ~0x01; // Filtre OFF
                f1A_val |= 0x02; // Chauffe ON
             }
          } else {
             if (filt) f1A_val |= 0x01;
             if (heat_ph > 0) f1A_val |= 0x02;
          }
          
          if (uvc_ph > 0) f1A_val |= 0x04;
          if (alert && (cycle < 5)) f1A_val |= 0x01;

          uint8_t f1A[4] = {0xA5, 0x1A, f1A_val, (uint8_t)(0xA5 + 0x1A + f1A_val)};
          id(uart_mb).write_array(f1A, 4);

          uint8_t s_val = (uint8_t)(setpoint - 30);
          uint8_t f1B[5] = {0xA5, 0x1B, (uint8_t)bubbles, s_val, (uint8_t)(0xA5+0x1B+bubbles+s_val)};
          id(uart_mb).write_array(f1B, 5);

          vTaskDelay(pdMS_TO_TICKS(100));
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
