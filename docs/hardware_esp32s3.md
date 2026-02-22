# Évolution vers ESP32-S3 (DEVKITC-1 N16R8)

Ce document décrit l’évolution **hardware** et **software** vers le module **ESP32-S3 WROOM-1-N16R8** sur carte **DEVKITC-1**, sans modifier les fichiers du contrôleur ESP32 classique. Pour le module **ESP32 classique**, voir `docs/hardware_esp32.md`.

---

## Référence module

- **Carte** : ESP32-S3 DEVKITC-1  
- **Module** : ESP32-S3-WROOM-1-N16R8 (ou équivalent N16R8)  
- **Flash** : 16 MB  
- **PSRAM** : 8 MB (Octal PSRAM intégré)  
- **ESPHome** : board `esp32-s3-devkitc-1`

---

## Fichiers dédiés (rien n’est écrasé)

| Fichier | Rôle |
|--------|------|
| `esphome/mspa-controller-s3.yaml` | Config ESPHome complète pour l’ESP32-S3 (même logique que le contrôleur ESP32, board et name différents). |
| `docs/hardware_esp32s3.md` | Ce document : brochure, différences, migration. |

Le contrôleur actuel **`esphome/mspa-controller.yaml`** et toute la doc existante restent inchangés.

---

## Brochage UART (identique à l’ESP32 classique)

Sur l’ESP32-S3, les GPIO 13, 14, 16 et 17 sont **libres** (non réservés au flash/PSRAM, non strapping). On garde le même câblage que sur l’ESP32 :

| Rôle | GPIO ESP32 | GPIO ESP32-S3 | Niveau |
|------|------------|---------------|--------|
| UART SPA RX | 16 | 16 | 3,3 V (level shifter 5 V → 3,3 V côté SPA) |
| UART SPA TX | 17 | 17 | 3,3 V |
| UART clavier RX | 14 | 14 | 3,3 V |
| UART clavier TX | 13 | 13 | 3,3 V |

- **Level shifter** 5 V ↔ 3,3 V inchangé (SPA et clavier en 5 V TTL).  
- **Alimentation** : le DEVKITC-1 peut être alimenté en USB ou via 5 V sur la broche dédiée ; vérifier le courant si tout part de l’USB.

---

## Logiciel (évolution soft)

- **Firmware** : utiliser **`esphome/mspa-controller-s3.yaml`**.
- **Secrets** : le même **`esphome/secrets.yaml`** que pour l’ESP32 (même clés). Penser à une **IP statique différente** pour l’ESP32-S3 dans `secrets.yaml` si les deux cartes coexistent sur le même réseau.
- **Compilation / flash** :
  ```bash
  py -m esphome compile esphome/mspa-controller-s3.yaml
  py -m esphome run esphome/mspa-controller-s3.yaml --device <IP_ESP32_S3>
  # ou --device COMx en USB
  ```
- **Nom d’appareil** : `mspa-controller-s3` (pour ne pas conflit avec `mspa-final-controller` sur l’ESP32).

---

## Différences matérielles utiles (ESP32 vs ESP32-S3)

| Point | ESP32 (actuel) | ESP32-S3 (N16R8) |
|-------|----------------|------------------|
| Flash | souvent 4 MB | 16 MB |
| PSRAM | optionnel, souvent 0 | 8 MB intégré |
| USB | USB-UART externe (CP2102/CH340) | USB native (USB-JTAG / USB-OTG selon broches) |
| Tension I/O | 3,3 V | 3,3 V (inchangé) |

Si tu utilises le **USB natif** du S3 pour la programmation, le port série peut être différent (ex. port USB-JTAG sous Windows). Le flash OTA reste possible une fois le WiFi configuré.

---

## En cas de boot loop ou souci PSRAM (N16R8)

Certaines cartes N16R8 nécessitent des options de build pour la PSRAM. Dans **`mspa-controller-s3.yaml`**, section `esp32:`, tu peux décommenter ou ajouter (selon la doc ESPHome / PlateformIO pour ton module) :

```yaml
esp32:
  board: esp32-s3-devkitc-1
  framework: { type: arduino }
  platformio_options:
    board_build.arduino.memory_type: qio_opi
    board_upload.flash_size: 16MB
```

À n’activer que si nécessaire (problème au démarrage ou flash non reconnue).

---

## Résumé

- **Hard** : même brochage UART (13/14/16/17) et level shifter ; alimentation et USB selon la carte DEVKITC-1.  
- **Soft** : fichier dédié **`mspa-controller-s3.yaml`** + même **secrets.yaml** ; compiler et flasher ce YAML sur l’ESP32-S3.  
- Aucun fichier existant (ESP32, doc protocole, eedomus, etc.) n’est modifié.
