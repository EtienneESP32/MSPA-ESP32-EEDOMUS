# Hardware – ESP32 classique (contrôleur actuel)

Ce document décrit le **matériel** et le **brochage** pour le contrôleur MSPA sur **ESP32** (carte type DevKit, WROOM, etc.). Même niveau de détail que `docs/hardware_esp32s3.md` pour l’ESP32-S3.

---

## Référence module

- **Cible** : ESP32 (SoC dual-core 240 MHz, WiFi + BLE).
- **Carte typique** : ESP32 DevKit (WROOM-32 ou équivalent), 4 MB flash, optionnel PSRAM.
- **ESPHome** : board `esp32dev` (ou `esp32-doit-devkit-v1` selon la carte).

---

## Fichiers dédiés (ESP32)

| Fichier | Rôle |
|--------|------|
| `esphome/mspa-controller.yaml` | Firmware contrôleur (pont UART + eedomus + UI). |
| `esphome/mspa-uart-sniffer.yaml` | Firmware d’audit (capture trames, sans modifier le bus). |

---

## Brochage UART

SPA et clavier sont en **5 V TTL** ; l’ESP32 est en **3,3 V**. Utiliser un **level shifter** 5 V ↔ 3,3 V sur les lignes RX/TX côté SPA et clavier.

| Rôle | GPIO | Sens | Niveau |
|------|------|------|--------|
| UART SPA RX | 16 | Entrée (SPA → ESP) | 3,3 V (depuis level shifter) |
| UART SPA TX | 17 | Sortie (ESP → SPA) | 3,3 V (vers level shifter) |
| UART clavier RX | 14 | Entrée (clavier → ESP) | 3,3 V |
| UART clavier TX | 13 | Sortie (ESP → clavier) | 3,3 V |

- **Baud rate** : 9600, 8N1.
- **Câblage** : voir schéma / description dans `docs/protocol_mspa.md` (architecture Man-in-the-Middle).

---

## Logiciel

- **Firmware** : `esphome/mspa-controller.yaml` (ou `mspa-uart-sniffer.yaml` pour le sniffer).
- **Secrets** : `esphome/secrets.yaml` (copier depuis `secrets.yaml.example`). Voir `docs/secrets_reference.md`.
- **Compilation / flash** :
  ```bash
  py -m esphome run esphome/mspa-controller.yaml --device <IP_ESP>
  # ou --device COM3 en USB
  ```

---

## Alimentation et USB

- Alimentation par **USB** (câble vers le PC ou alimentation 5 V) ou par **5 V** sur la broche dédiée de la carte selon le modèle.
- Programmation : **USB-UART** (CP2102, CH340, etc.) selon la carte ; le port apparaît comme COMx (Windows) ou `/dev/ttyUSBx` (Linux).

---

## Résumé

- **Hard** : ESP32 + level shifter 5 V / 3,3 V ; UART SPA (16/17), UART clavier (14/13).
- **Soft** : `mspa-controller.yaml` ou `mspa-uart-sniffer.yaml` ; même `secrets.yaml` pour le contrôleur.

Pour la variante **ESP32-S3**, voir `docs/hardware_esp32s3.md`.
