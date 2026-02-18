# Projets similaires (spa / hot tub + ESP)

Ce document recense des projets open source de contrôle de spa / jacuzzi via ESP (ESP32/ESP8266), utiles pour comparaison ou inspiration. **Notre projet** cible le **MSPA série D** (protocole 0xA5, UART 9600) et **eedomus** ; les protocoles Balboa / Intex / Watkins diffèrent.

---

## Contrôle spa par UART / bus

| Projet | Matériel | Protocole / marque | Intégration | Lien |
|--------|----------|--------------------|-------------|------|
| **esp32_balboa_spa** | ESP32 | Balboa (UART) | WiFi, module contrôle | [NorthernMan54/esp32_balboa_spa](https://github.com/NorthernMan54/esp32_balboa_spa) |
| **esp32-balboa-spa** | ESP32 | Balboa | Rust, libs + émulateurs | [jasta/esp32-balboa-spa](https://github.com/jasta/esp32-balboa-spa) |
| **esp8266_spa** | ESP8266 | Balboa BP2100G0 | Base de nombreux forks | [cribskip/esp8266_spa](https://github.com/cribskip/esp8266_spa) |
| **ESPHomeSpa** | ESP8266 / ESP32 | Balboa | **ESPHome** (sans MQTT) | [jrowny/ESPHomeSpa](https://github.com/jrowny/ESPHomeSpa) |

- Les projets **Balboa** parlent un protocole propriétaire différent du MSPA 0xA5 ; le câblage et les trames ne sont pas compatibles.
- **ESPHomeSpa** est le plus proche en approche : ESPHome natif, pas de MQTT obligatoire.

---

## Autres marques / modèles avec ESPHome

| Projet / doc | Modèle | Remarque |
|--------------|--------|----------|
| **Intex Pure Spa (SB-H20)** | Intex | ESP8266, ESPHome, doc [esphome-intexsbh20](https://esphome-intexsbh20.piitaya.fr/) |
| **Watkins IQ2020** | Hot Spring / Tiger River | ESP32, RS485, fiche [devices.esphome.io](https://devices.esphome.io/devices/Watkins-IQ2020) |
| **Lay-Z-Spa** | Lay-Z-Spa (gonflable) | [sanyatuning/hot-tub](https://github.com/sanyatuning/hot-tub) |
| **Hot tub monitoring** | Générique | ESPHome + Home Assistant, monitoring (temp, pompes, etc.) – ex. [jnsgr.uk](https://jnsgr.uk/2024/11/hot-tub-monitoring-with-esphome/) |

---

## Positionnement de notre projet (MSPA-ESP32-EEDOMUS)

- **Marque** : MSPA (série D), pas Balboa ni Intex.
- **Protocole** : trames 4 octets `0xA5` \| ID \| Data \| Checksum (voir `docs/protocol_mspa.md`).
- **Rôle** : pont UART man-in-the-middle (clavier ↔ moteur) + push vers **eedomus** (HTTP API), pas Home Assistant ni MQTT.
- **Firmwares** : `mspa-controller.yaml` (production), `mspa-uart-sniffer.yaml` (audit / reverse engineering).

Pour comparer le **format des trames** ou le **câblage**, les docs des projets Balboa peuvent aider sur l’idée générale (UART, level shifter), mais les IDs et la structure restent spécifiques MSPA.
