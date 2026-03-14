# Historique des changements (MSPA ESP32)

### 2026-03-14 – Régression Bulles (v3.6.1)
- **Régression identifiée** : En v3.6.0, le `set_action` du select "Mode bulles" envoyait la commande via `pending_uart_id` (Sniper sur Poll 0x0D). Le moteur n'exécute pas `0x03` par ce canal — les bulles étaient silencieusement ignorées.
- **Correction** : Retour à l'écriture **UART directe** (`uart_spa.write_array()`) pour les bulles `0x03`. Validé terrain comme seul mécanisme fonctionnel.
- **Documentation** : `docs/logic_spec.md §4` réécrit avec règles immuables par mode. En-tête du YAML mis à jour avec tableau récapitulatif Mode 1/2/3.
- **Règle ajoutée** : `0x03` (Bulles) et `0x04` (Consigne) = Mode 1 DIRECT uniquement. `0x02` (Filtration) = Mode 2 MITM uniquement. `0x01` et `0x19` = Mode 3 Sniper uniquement.

### 2026-03-14 – Bug-Fix Release (v3.6.0)
- **Fix A2 — p_step global** : `p_step` (machine d'état Sniper) promu en variable globale ESPHome. Chaque activation/désactivation de Chauffage ou UVC remet p_step à 0, évitant les séquences d'injection hybrides en cas de changement d'ordre entre deux Polls.
- **Fix A4 — Injection consigne** : le slider "Temperature consigne" envoie désormais la trame `A5 04 [Temp×2] CS` au moteur SPA. La consigne était précédemment un widget read-only.
- **Fix A7 — Sniffer sémantique** : le bit 3 de la trame `0x1A` est désormais nommé `Pret` (au lieu de `Idle`) dans le sniffer, aligné sur `protocol_mspa.md` et le binary_sensor du contrôleur.

## 2026-02 – Nettoyage dépôt (BLE abandonné, configs de test)

- **BLE** : piste abandonnée ; suppression de `ble-spa-sniff.yaml` et de toute référence au BLE.
- **Configs de test** : suppression des YAML de phase (clavier seul, SPA seul, pont transparent) à la racine ; le dépôt ne garde que le contrôleur (`esphome/mspa-controller.yaml`) et le sniffer (`esphome/mspa-uart-sniffer.yaml`).
- **Doc** : `TEST_PROTOCOLE.md` mis à jour (phases validées décrites sans références aux configs supprimées). Correction `level: WARNING` → `WARN` dans le contrôleur (compatibilité ESPHome).

---

---

- **Zéro Lissage** : Suppression des temporisations de 20s. L'UI affiche la vérité brute du bus UART instantanément.

### 2026-03-08 – Correctif "Magic Bytes" (v3.4.16)
- **Détection** : Identification d'un conflit majeur entre les trames de 5 octets (`0x00`) and le composant `ota:` d'ESPHome.
- **Action** : Optimisation du buffer UART pour prioriser le décodage applicatif sur les alertes OTA.
- **Correction** : Séparation stricte de la Consigne (D2 de 1B) et des Flags Moteur (1A) pour éviter que la consigne ne bloque l'UI sur "Chauffage ON".

### 2026-03-11 – Architecture "Sync-Sniper" (v3.5.0)
- **Détection** : Échec aléatoire ou total des commandes One-Shot (Chauffe, UVC) dû à une injection asynchrone ratant la fenêtre d'écoute de la carte mère.
- **Réécriture** : Abandon du timer asynchrone (`now - last_p`). Injection synchronisée **strictement sur la trame Poll (`0x0D`)** du clavier (mode Sniper).
- **Correctif UI** : Correction du bitmask UVC (`0x10` -> `0x04`) suite à exhumation des logs UART "en eau".
- **Prévention réseau** : Buffer UART augmenté à `1024` octets pour résister aux latences d'appel HTTP de l'API Eedomus (passage `eedomus_enabled: true` prêt).

---

### Contexte

Après une perte des fichiers sources (restauration depuis Git), le code du **contrôleur** et du **sniffer** a été reconstitué à partir de la discussion de développement. Les secrets ont été recréés via `secrets.yaml` (hors dépôt) et `secrets.yaml.example` + `docs/secrets_reference.md`.

---

### Contrôleur (`esphome/mspa-controller.yaml`)

- **Boot** : consigne initiale 40 °C ; délai 90 s avant le premier probe eedomus (UI + sockets stables).
- **Stabilité** :
  - `api: reboot_timeout: 0s` (plus de reboot « no clients » ; eedomus pilote l’ESP via HTTP / serveur web, pas l’API native).
  - `api: max_connections: 4`, `listen_backlog: 2` pour libérer des sockets (UI + requêtes eedomus, moins de saturation).
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
- **`docs/depannage_logs.md`** : section « Améliorer le responsive de l’UI et limiter la saturation » (réglages API, un seul onglet, option probe 180s, prod sans web_server).
- **`esphome/secrets.yaml.example`** : modèle avec toutes les clés (dont optionnel IP fixe) ; renvoi vers `docs/secrets_reference.md`.
- **README.md** : mise à jour pour refléter la doc et l’option IP fixe.

---

### Utilisation

- **Secrets** : copier `esphome/secrets.yaml.example` en `esphome/secrets.yaml`, remplir les valeurs (voir `docs/secrets_reference.md`). Ne jamais committer `secrets.yaml`.
- **Compilation / OTA** :
  - Contrôleur : `py -m esphome run esphome/mspa-controller.yaml --device <IP_ESP>`
  - Sniffer : `py -m esphome run esphome/mspa-uart-sniffer.yaml --device <IP_ESP>`
