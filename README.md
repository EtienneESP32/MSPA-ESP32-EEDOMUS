# Contrôleur MSPA (ESP32)

Pont UART **man-in-the-middle** entre le clavier et le moteur d’un spa MSPA (série D), avec pilotage domotique (eedomus) et interface Web.

## Contenu du dépôt

| Dossier / Fichier | Rôle |
|-------------------|------|
| `docs/` | Documentation : protocole, règles logiques, plan de tests (v3.5.0 révisée) |
| `docs/archive/` | Archives des anciens logs UART et configurations de tests obsolètes |
| `esphome/mspa-controller.yaml` | Firmware principal (pont + commandes + eedomus) – ESP32 classique (Architecture V3.5 Sync-Sniper) |
| `esphome/mspa-controller-s3.yaml` | Variante **ESP32-S3** (DEVKITC-1 N16R8) – voir `docs/hardware_esp32s3.md` |
| `esphome/mspa-simulator.yaml` | **Simulateur MSPA** (v1.5.9) – Banc de test matériel pour injection UART |
| `esphome/mspa-uart-sniffer.yaml` | Firmware d’audit : capture des trames UART (sans modifier le bus) |

## Matériel

- **ESP32** (UART 9600 bps) – brochure : `docs/hardware_esp32.md`
- **Level shifter** 5V ↔ 3,3 V (SPA et clavier en 5 V TTL)
- Câblage : voir `docs/protocol_mspa.md`

## Documentation

- **Protocole** : `docs/protocol_mspa.md` (structure des trames, IDs, checksum)
- **Règles logiques** : `docs/logic_spec.md` (priorité, lock, source de vérité)
- **Tests** : `docs/test_plan.md` (phases avec/sans eau, sniffer)
- **Config eedomus** : `docs/config_eedomus.md` (périphériques, correspondance ESP↔Eedomus, création des périphériques)
- **Hardware ESP32** : `docs/hardware_esp32.md` (brochage, level shifter, firmware dédié)
- **Hardware ESP32-S3** : `docs/hardware_esp32s3.md` (évolution N16R8, migration, `mspa-controller-s3.yaml`)
- **Simulateur (Banc de test)** : `docs/simulator_user_guide.md` (guide d'utilisation du module de simulation v1.5.9)
- **Secrets** : `docs/secrets_reference.md` (liste complète des clés ; recréer `secrets.yaml` en local)
- **Projets similaires** : `docs/projets_similaires.md` (Balboa, ESPHomeSpa, Intex, Watkins – comparaison)
- **Changelog** : `docs/CHANGELOG.md` (Passage à l'architecture "Sync-Sniper" v3.5.0, Buffer UART 1024, fix UVC)
- **Dépannage / logs** : `docs/depannage_logs.md` (erreurs HTTP, OTA, eedomus)
- **Cursor (workspace, nouveau projet)** : `docs/cursor_workspace.md` (workspace vs dossier, créer/ouvrir un autre projet)

## Première utilisation (secrets)

Les firmwares dans `esphome/` utilisent des **secrets** (WiFi, eedomus). Aucun mot de passe n’est dans le dépôt.

1. Copie `esphome/secrets.yaml.example` en **`esphome/secrets.yaml`**
2. Ouvre `esphome/secrets.yaml` et remplace les valeurs par les tiennes (SSID, mot de passe WiFi, optionnellement eedomus)
3. **Ne committe jamais** `secrets.yaml` (il est dans `.gitignore`)

## Compilation / flash

```bash
# Contrôleur – OTA (remplacer <IP_ESP> par l’IP actuelle de l’ESP, ou utiliser le port série)
py -m esphome run esphome/mspa-controller.yaml --device <IP_ESP>

# Contrôleur – USB (si OTA indisponible)
py -m esphome run esphome/mspa-controller.yaml --device COM3

# Sniffer
py -m esphome run esphome/mspa-uart-sniffer.yaml --device <IP_ESP>
# ou --device COM3
```

L’IP de l’ESP peut être en DHCP ou fixe (voir `secrets.yaml.example` et `manual_ip` dans le contrôleur). Adapter `<IP_ESP>` ou le port COM selon ton réseau. Aucune donnée sensible n’est dans le dépôt.

## Dépannage et logs

En cas de **Failed to create socket** ou **ESP_ERR_HTTP_CONNECT** (probe eedomus), ou **httpd_accept_conn: error (23)** : voir `docs/depannage_logs.md` (analyse des causes et pistes de résolution).

## Licence

À définir (ex. MIT). Voir fichier LICENSE si présent.
