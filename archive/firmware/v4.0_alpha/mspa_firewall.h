#include "esphome.h"

class MspaFirewall : public Component {
 public:
  UARTDevice *uart_spa; // Vers le moteur
  UARTDevice *uart_kbd; // Vers le clavier

  bool lock_keypad = false;
  uint32_t last_mb_frame_ms = 0;

  MspaFirewall(UARTDevice *spa, UARTDevice *kbd) : uart_spa(spa), uart_kbd(kbd) {}

  void setup() override {
    ESP_LOGI("custom", "MspaFirewall v4.0 Alpha - Mode Transparent initialisÃ©");
  }

  void loop() override {
    // 1. Passage MOTEUR -> CLAVIER (Toujours transparent)
    while (uart_spa->available()) {
      uint8_t c = uart_spa->read();
      uart_kbd->write(c);
      // Analyser l'état ici (id=0x08, 0x1A) sans bloquer
    }

    // 2. Passage CLAVIER -> MOTEUR (Filtrage Sélectif)
    while (uart_kbd->available()) {
      uint8_t c = uart_kbd->read();
      
      if (lock_keypad) {
        // Logique simplifiée de blocage (à affiner avec détection de préambule A5)
        // Pour l'instant, on laisse tout passer pour la Release 4.0
        uart_spa->write(c);
      } else {
        uart_spa->write(c);
      }
    }
  }
};
