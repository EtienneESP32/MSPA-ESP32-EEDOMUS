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

## Compilation / flash

```bash
# Contrôleur (usage normal)
esphome compile esphome/mspa-controller.yaml
esphome run   esphome/mspa-controller.yaml --device COM3

# Sniffer (capture des trames uniquement)
esphome compile esphome/mspa-uart-sniffer.yaml
esphome run   esphome/mspa-uart-sniffer.yaml --device COM3
```

## Licence

À définir (ex. MIT). Voir fichier LICENSE si présent.
