# Spécifications logiques du contrôleur MSPA (ESP32)

## 1. Hiérarchie des commandes (synchronisation)

- **Entité numérique** : eedomus + interface Web ESP32 = un seul organe de commande. Toute modification sur l’un est répercutée sur l’autre.
- **Priorité** : "Dernier qui parle a raison".
  - Ordre via eedomus/UI → ESP32 modifie le bus UART.
  - Bouton physique sur le clavier → ESP32 détecte, met à jour son état, informe eedomus.

## 2. Source de vérité (retour d’état) Absolute

- Le moteur ESP32 est en position d'**écoutes absolue** (Pass-Through).
- L’état affiché sur l'UI et envoyé à Eedomus n'est mis à jour **que lorsque** le Spa envoie son état physique dans les trames `0x1A`.
- **Bulles (Spécificité)** : L'ID `0x1A` n'est **PAS** fiable pour le retour d'état des bulles sur ce modèle. La seule source de vérité absolue est l'ID **`0x1B`** (trame 5 octets).
- Si le SPA allume ou éteint un composant de façon autonome (cycle de nettoyage, thermostat atteint), l'interface Web s'adaptera passivement pour refléter la réalité. L'ESP **ne combat jamais** les décisions autonomes du SPA.

## 3. Mode Lock (verrouillage clavier)
*(Note: Fonctionnalité optionnelle non intégrée dans le firmware de base Absolute Sync pour l'instant).*
- **Piloter par** : eedomus ou l’UI.
- **Principe** : Si implémenté, interceptés les trames `0x01` du clavier et les remplacer par `0x00`.

## 4. Stratégie d'Injection : Les 3 Modes (validés terrain)

> [!IMPORTANT]
> Ces 3 modes sont **figés par les tests terrain**. Ne jamais changer de mode pour une commande sans tests complets. Toute modification doit être documentée dans le CHANGELOG.

L'ESP32 interagit avec le Spa selon 3 mécaniques distinctes, **chacune assignée à des commandes spécifiques** :

### Mode 1 — Direct Asynchrone (Bulles `0x03`, Consigne `0x04`)

- **Implémentation** : `uart_spa.write_array()` direct dans le `set_action` du composant ESPHome.
- **Pas de `pending_uart_id`**. Pas d'attente du Poll `0x0D`.
- **Pourquoi** : Le moteur accepte ces commandes à tout moment. La réactivité est immédiate.
- ⚠️ **Règle immuable** : Ne JAMAIS envoyer `0x03` ou `0x04` via `pending_uart_id` (Sniper). Validé terrain — cela casse les bulles.

### Mode 2 — MITM Interception (Filtration `0x02`)

- **Implémentation** : Dans le `on_loop`, le clavier envoie `0x02 OFF` périodiquement. Si l'ESP veut forcer ON, il modifie l'octet Data à la volée avant de le transmettre au moteur, et recalcule le checksum.
- **Pas de `pending_uart_id`**. Le switch `sw_f` contrôle l'état via MITM uniquement.
- ⚠️ **Règle immuable** : Ne JAMAIS envoyer `0x02` via `pending_uart_id` (Sniper). Le MITM seul suffit.

### Mode 3 — Sniper Synchrone sur Poll `0x0D` (Chauffage `0x01`, UVC `0x19`)

- **Implémentation** : L'ordre est stocké dans `pending_uart_id` / `pending_uart_val`. Dès que le `on_loop` détecte le Poll `0x0D` du clavier, il injecte la commande sur `uart_spa` immédiatement, avant de relayer le Poll.
- **Cascade automatique** : si la pompe est OFF et que la commande est `0x01` ou `0x19` ON, l'ESP démarre d'abord la pompe (`0x02`) au Poll suivant, puis injecte la commande cible au Poll d'après.
- ⚠️ **Règle immuable** : Seuls `0x01` et `0x19` passent par le Sniper. `p_step` est un global ESPHome (id: `p_step`) — réinitialisé à 0 à chaque nouvel ordre pour éviter les états hybrides.


## 5. Télémétrie température

- Envoi vers eedomus uniquement en cas de **variation significative** ou **heartbeat** fixe, pour limiter le trafic et la base de données.
