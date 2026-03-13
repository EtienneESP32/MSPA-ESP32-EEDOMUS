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

## 4. Stratégie d'Injection : Les 3 Modes (V3.5.0)

L'ESP32 interagit avec le Spa selon 3 mécaniques matérielles distinctes :

1. **Mode 1 : Bulles et Consigne (Trame Étendue 0x1B)**
   - Canal asynchrone indépendant : L'ESP32 modifie ou envoie directement la trame de 5 octets sans attendre d'autorisation du moteur.
2. **Mode 2 : Filtration (Interception Pure "MITM")**
   - Le clavier envoie périodiquement l'état de la pompe (`0x02`). Si l'Eedomus demande l'allumage, l'ESP32 attend la trame `OFF` du clavier, modifie l'octet en `ON` à la volée, recalcule le checksum et transmet au moteur. Le timing est virtuellement parfait.
3. **Mode 3 : Chauffe et UVC (Mode "Sniper" Synchrone)**
   - Ces commandes sont ponctuelles ("One-Shot") et nécessitent que la pompe tourne.
   - Si la pompe est OFF, l'ESP force l'allumage de la pompe ("Cascade").
   - L'ordinateur du Spa n'écoute les commandes que juste après le "Poll" du clavier (`0x0D` envoyé toutes les 9s). L'ESP garde l'ordre en mémoire (`pending_uart_id`) et l'injecte dans la milliseconde qui suit la détection du `0x0D` pour garantir son exécution.

## 5. Télémétrie température

- Envoi vers eedomus uniquement en cas de **variation significative** ou **heartbeat** fixe, pour limiter le trafic et la base de données.
