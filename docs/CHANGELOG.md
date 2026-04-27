# Historique des changements (MSPA ESP32)

### 2026-04-27 — Correction Asymétrie & Offset Bible (v6.9.16-LABO)
- **Protocole (mspa_uart.h)** : Correction de l'asymétrie de réception de la consigne (Offset 30 / d2 + 30.0f) pour conformité avec le Simulateur et la Bible.
- **Diagnostic** : Intégration des capteurs de performance (Loop Time, Heap, Min Heap) pour évaluation de la charge CPU/RAM.
- **Version** : Unification des versions LABO et Controller en v6.9.16.

### 2026-04-24 — Restauration "Heritage Core" & Stabilisation Sockets (v6.9.8-STABLE)
- **Restauration Moteur v6.7.8** : Retour à la logique UART ultra-stable (Heritage).
- **Passthrough Immédiat** : Renvoi des octets sans buffering pour éliminer le jitter et les collisions.
- **Protection Mutex** : Utilisation d'un sémaphore FreeRTOS pour interdire les écritures simultanées sur les ports UART.
- **Cadence eedomus 15s** : Retour au délai de sécurité historique pour garantir la fermeture des sockets HTTPS.
- **Watchdog HTTP 30s** : Libération forcée du verrou `http_busy` en cas de timeout cloud.
- **Sniper v6.7.8** : Retour à l'injection "Pulse" simple avec logique de Retry (3x).

### 2026-04-16 — Filter Reset & Sync Optimization (Controller v6.3.7 / Simulator v1.6.9)
- **Controller (Filter Reset)** : Création d'un bouton dédié "Action Reset Alerte" qui envoie une rafale de 4 secondes pour acquitter l'alerte de changement de filtre à distance.
- **Simulator (v1.6.9)** : Désaccouplage de la détection d'appui long de la barrière anti-vibration (500ms) pour permettre la capture haute fréquence des trames de reset.
- **Protocole** : Passage de la méthode `inject_cmd` en public dans `mspa_uart.h` pour permettre l'appel direct depuis le YAML.
- **Correction** : Rétablissement de la synchronisation de la consigne de température dans la v1.6.9.

### 2026-04-04 — Alerte eedomus & Master Sync (v6.3.6-STABLE)
- **eedomus** : Implémentation de la remontée d'alerte automatique du filtre (ID 3536562).
- **Welcome Sync** : Script de synchronisation exhaustive (8 entités) au démarrage avec délai de 500ms pour éviter la surcharge.
- **Sécurité** : Point d'accès `MSPA-RECOVERY` désormais protégé par mot de passe (via secrets).
- **Logique d'état** : Harmonisation des noms d'entités pour une compatibilité 100% sans changer les réglages eedomus.
- *Projet maintenu par Etienne - Stabilité Master validée le 04/04/2026 (Sync & Security OK). Version 6.3.6-STABLE.*

### 2026-03-28 — Audit & Diagnostic (v6.3.5-STABLE)
- **API Native ESPHome** : Activation du port API (6053) avec `reboot_timeout: 0s`. Permet le monitoring et les logs à distance sans risque de redémarrage cyclique.

### 2026-03-28 — Release Finale (v6.3.4-STABLE)
- **UART Firewall (Production)** : Filtrage sélectif actif. Blocage des commandes physiques en mode "Verrouillage". Heartbeat `0x0D` préservé.
- **Miroir d'état (100% Natif)** : L'interface suit strictement le bus UART, pas de forcage fictif.
- **Auto-Correction Sniper v2** : Ré-essais automatiques en cas de perte de trame.
- **Alerte Filtre (DIAG)** : Détection intelligente du clignotement filtre via `0x1A/0x08`.

### 2026-03-27 — Simulateur Labo (v1.6.0-STABLE)
- **Cycle des Bulles Natif** : Implémentation du cycle `0 -> 2 -> 3 -> 1 -> 0`.
- **Zéro Backdoor** : Suppression des accès directs à la mémoire. Tout passe par le bus UART pour une simulation 100% fidèle.
- **Bridge UART Dédié** : Support complet des trames 5-octets (`0x1B`).
