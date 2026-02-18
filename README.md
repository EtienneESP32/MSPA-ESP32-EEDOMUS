# Contrôleur MSPA (ESP32)

Pont UART **man-in-the-middle** entre le clavier et le moteur d’un spa MSPA (série D), avec pilotage domotique (eedomus) et interface Web.

## Contenu du dépôt

| Dossier / Fichier | Rôle |
|-------------------|------|
| `docs/` | Documentation : protocole, règles logiques, plan de tests |
| `esphome/mspa-controller.yaml` | Firmware principal (pont + commandes + eedomus) |
| `esphome/mspa-uart-sniffer.yaml` | Firmware d’audit : capture des trames UART (sans modifier le bus) |

## Matériel

- **ESP32** (UART 9600 bps)
- **Level shifter** 5V ↔ 3,3 V (SPA et clavier en 5 V TTL)
- Câblage : voir `docs/protocol_mspa.md`

## Documentation

- **Protocole** : `docs/protocol_mspa.md` (structure des trames, IDs, checksum)
- **Règles logiques** : `docs/logic_spec.md` (priorité, lock, source de vérité)
- **Tests** : `docs/test_plan.md` (phases avec/sans eau, sniffer)
- **Secrets** : `docs/secrets_reference.md` (liste complète des clés ; recréer `secrets.yaml` en local)
- **Changelog** : `docs/CHANGELOG.md` (restauration, stabilisation, sniffer)

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

## Licence

À définir (ex. MIT). Voir fichier LICENSE si présent.
