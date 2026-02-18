# Historique des changements (MSPA ESP32)

## 2026-02 – Restauration et stabilisation

### Contexte

Après une perte des fichiers sources (restauration depuis Git), le code du **contrôleur** et du **sniffer** a été reconstitué à partir de la discussion de développement. Les secrets ont été recréés via `secrets.yaml` (hors dépôt) et `secrets.yaml.example` + `docs/secrets_reference.md`.

---

### Contrôleur (`esphome/mspa-controller.yaml`)

- **Boot** : consigne initiale 40 °C ; délai 90 s avant le premier probe eedomus (UI + sockets stables).
- **Stabilité** :
  - `api: reboot_timeout: 0s` (plus de reboot « no clients » ; eedomus pilote l’ESP via HTTP / serveur web, pas l’API native).
  - `wifi: power_save_mode: none` ; `ota: version: 2` ; `safe_mode: reboot_timeout: 10min`.
  - `http_request: timeout: 1s` ; probe eedomus 120 s si joignable (dev avec UI), 5 min si injoignable.
- **Boutons (Filtration, Chauffage, UVC)** : écriture UART directe depuis les actions des switches (trame 0xA5 envoyée au SPA).
- **Verrouillage clavier** : Lock ON → seul le flux **clavier → SPA** est bloqué ; eedomus et l’UI peuvent toujours piloter le SPA (voir `docs/logic_spec.md`).
- **Boucle** : max 2 trames par cycle, deux `yield()` (UI responsive), checksum sur trames SPA (0x1A), `opt.size()` pour bulles, `has_state` pour consigne.
- **Bulles / consigne** : envoi UART direct dans `set_action` (select) et `set_action` (number).
- **IP fixe** : prise en charge via secrets `static_ip`, `gateway`, `subnet` et `manual_ip` dans le WiFi.

Références : `docs/protocol_mspa.md`, `docs/logic_spec.md`, `docs/config_eedomus.md`.

---

### Sniffer (`esphome/mspa-uart-sniffer.yaml`)

- **Pont 100 % transparent** : aucune injection (ex. plus d’injection 0x03) ; simple relais des octets entre clavier et SPA.
- **Préfixes dans les logs** :
  - `[BOOT]` : première trame après démarrage.
  - `[+Nms]` : délai en ms depuis la trame précédente (à partir de la 2ᵉ trame).
  - `[apres 0x0D]` : trame qui suit un poll clavier (ID 0x0D).
- **IDs nommés** : 0x08 → `0x08(SPA)`, 0x0D → `Poll` (voir `docs/protocol_mspa.md`).
- **Globals** : `last_trame_ms`, `last_rid`, `first_trame_done` pour le timing et les préfixes.

Références : `docs/protocol_mspa.md`, `docs/test_plan.md`.

---

### Documentation

- **`docs/secrets_reference.md`** : liste de toutes les clés utilisées dans le code (contrôleur + sniffer), avec fichier et usage. À utiliser pour recréer `secrets.yaml` en local.
- **`esphome/secrets.yaml.example`** : modèle avec toutes les clés (dont optionnel IP fixe) ; renvoi vers `docs/secrets_reference.md`.
- **README.md** : mise à jour pour refléter la doc et l’option IP fixe.

---

### Utilisation

- **Secrets** : copier `esphome/secrets.yaml.example` en `esphome/secrets.yaml`, remplir les valeurs (voir `docs/secrets_reference.md`). Ne jamais committer `secrets.yaml`.
- **Compilation / OTA** :
  - Contrôleur : `py -m esphome run esphome/mspa-controller.yaml --device <IP_ESP>`
  - Sniffer : `py -m esphome run esphome/mspa-uart-sniffer.yaml --device <IP_ESP>`
