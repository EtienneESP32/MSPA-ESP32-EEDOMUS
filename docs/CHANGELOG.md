# Historique des changements (MSPA ESP32)

### 2026-05-11 — État Fonctionnel "Clean State" (v7.5.9-STABLE)
- **Décodage Sémantique** : Implémentation du verrouillage (Latching) basé sur le relais moteur. L'icône de filtration ne clignote plus dans l'UI lors d'une alerte (elle reste stable à OFF).
- **Sniper Déchaîné** : Déclenchement du mécanisme d'injection sur la trame `0x1A`. Gain massif en réactivité pour les commandes de chauffe et filtration.
- **Bugfix Sniper Fight** : Correction du conflit où le Sniper tentait de "tuer" l'alerte filtre, libérant ainsi la bande passante pour la commande de chauffe.
- **Sanctuary** : Nouveau commit de sanctuarisation pour cette version charnière.


### 2026-05-10 — Sécurisation & Résilience Bus (v7.5.6-STABLE)
- **Hardenization Core** : Augmentation de la pile (Stack) de la tâche UART à **8192 octets**. Prévient les crashs de type "Stack Overflow" lors du traitement de trames complexes ou de tempêtes de logs.
- **Optimisation Performance** : Désactivation des logs `DEBUG` (0x08 et 0x1A) dans le flux UART haute fréquence. Gain de cycles CPU significatif et réduction de la latence du bus.
- **Robustesse Labo** : Passage du niveau de log global à `INFO` dans le YAML pour éviter la saturation des sockets réseau lors des tests de stress (Ghosting).
- **Maintenance** : Mise à jour de la bannière de démarrage et de l'identification logicielle pour suivi rigoureux des versions.


### 2026-04-29 — Stabilisation Réseau & Priorité (v6.9.18-PROD)
- **Nettoyage UI (PROD)** : Masquage des capteurs de diagnostic (Lien Clavier, Lien Moteur, Démarré le) de l'interface Web pour alléger la charge du serveur local.
- **Stabilité au Boot** : Ajout d'un délai de grâce de 30 secondes avant le démarrage de la file d'attente Eedomus pour laisser l'ESP (NTP, Web Server, Wi-Fi) se stabiliser sans saturer les sockets.
- **Correction File d'Attente C++** : Modification de `enqueue_eedomus` (utilisation de `push_front`) pour que les messages prioritaires (Alertes, Changements d'état) passent instantanément en tête de file devant les températures, tout en respectant l'intervalle de sécurité de 15s.
- **Compatibilité Eedomus (Heritage)** : Restauration scrupuleuse des noms historiques des composants (ex: `"Action Reset Alerte"`) pour éviter les erreurs HTTP 404 qui provoquaient un pilonnage de l'Eedomus et la saturation immédiate des connexions.
- **Heartbeat Température** : Ajout d'un intervalle forcé d'1 heure (`interval: 1h`) pour pousser la température vers l'Eedomus même sans variation, garantissant un contrôle de "vie" du système.
- **LIMITATIONS & APPRENTISSAGES** : 
  - *Sockets ESP32* : Limite physique très basse (8-10 max). Garder la page Web ouverte consomme des sockets et peut bloquer les requêtes sortantes (`Error 23: failed to create socket`).
  - *Keep-Alive* : Maintenir les connexions HTTP ouvertes sature l'ESP. Les 15s de délai entre envois sont vitales.
  - *Erreurs 404 Eedomus* : Une URL invalide côté Eedomus (suite à un renommage de composant) entraîne des tentatives de reconnexion agressives qui tuent littéralement le réseau de l'ESP en quelques secondes.


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
