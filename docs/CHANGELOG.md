# Historique des changements (MSPA ESP32)

### 2026-05-12 — DIAMOND FINAL (v7.5.37)
- **Phase 2 (Double FIFO)** : Séparation priorisée des flux Action et Status.
- **Phase 1 (Non-Dictature)** : Synchronisation clavier physique / cibles.
- **Phase 3 (Hybride)** : Filtre d'Enveloppe (3s) + Priorité Moteur Physique.
- **Stabilisation Sockets** : Nettoyage radical des processus et protection Socket Guard (48K).

### 2026-05-11 — Atomic Sanctuary & Lock-Free Core (v7.5.26-ULTRA-RESILIENT)
- **Architecture Lock-Free** : Suppression totale des verrous Mutex sur le relais d'octets UART. Le Cœur 1 (Sanctuaire) est désormais totalement indépendant du Cœur 0 (Réseau).
- **Variables Atomiques (`std::atomic`)** : Migration de tous les états partagés (Commandes Sniper, Températures, Flags) vers des types atomiques. Élimine tout risque de blocage ou de "Race Condition" entre les deux cœurs.
- **Réactivité Sniper < 1s** : Validation d'un temps de réaction d'environ 1 seconde entre le clic UI et l'injection physique sur le bus, même lors de fortes latences WiFi.
- **Immunité Réseau** : Le bus UART ne peut plus être figé par des timeouts HTTP ou des défaillances DNS. Le "Lien Moteur" et le "Lien Clavier" restent stables en toutes circonstances.
- **Certification GOLD** : Version finale certifiée pour le déploiement en environnement bruité ou instable.

### 2026-05-11 — Résilience Totale & Socket Guard (v7.5.25-LABO-RESILIENT)
- **Socket Guard (Anti-Asphyxie)** : Vérification systématique du Heap avant chaque envoi HTTP. Seuil de sécurité à **45 Ko**. En cas de mémoire basse, le push est annulé pour garantir la survie de l'accès Web et OTA.
- **Dédoublonnage Intelligent (Queue Collapse)** : Implémentation d'un algorithme de nettoyage de file d'attente. Si une valeur change avant d'être envoyée, l'ancienne est écrasée. Empilement de requêtes vers Eedomus impossible (max 1 requête par périphérique dans la file).
- **Throttle Dynamique (Gentle Mirror)** : Réduction de l'intervalle de sécurité à **5 secondes** entre deux requêtes (contre 15s auparavant), géré de manière fluide par le Cœur 0 sans bloquer l'exécution.
- **Fail-Safe Timeout** : Réduction du verrouillage HTTP de 30s à **10s** en cas de blocage réseau (TCP Hang), libérant ainsi les ressources plus rapidement.
- **Boot Sync Optimisé** : Suppression des délais de 15s dans le YAML. La synchronisation initiale est maintenant gérée par la file d'attente C++, permettant une mise à jour exhaustive en ~40 secondes sans risque de crash.
- **Lien Eedomus UI** : Ajout d'un interrupteur logiciel dans l'interface Web pour couper instantanément toute communication sortante sans reflasher.
- **Certification "Worst Case"** : Validé avec succès face à une Eedomus injoignable (IP fantôme) : Uptime stable, Heap à 145 Ko, Interface Web fluide.

### 2026-05-11 — Master Sanctuary (v7.5.23-BIBLE-ULTRA)
- **User Priority Rule** : Chaque action UI réinitialise instantanément les flags de Ghosting pour le composant concerné. Empêche le verrouillage indésirable observé lors de clics rapides.
- **Sniper Persistence** : Augmentation du nombre de réessais à **10 tentatives** pour surmonter les transitions de bus bruité.
- **Robustesse Labo** : Élimination du conflit YAML "Refus Filtration OFF" par synchronisation forcée des états.

### 2026-05-11 — Restauration Décentralisée (v7.5.22-BIBLE-ULTRA)
- **Architecture v7.5.14** : Retour à la distribution de la charge réseau entre les deux cœurs. Le Core 1 gère ses propres `publish_state` filtrés, libérant le Core 0 pour le Web.
- **Loop Safety (Anti-Freeze)** : Implémentation d'une bride de 64 octets par cycle UART et timeout Mutex de 10ms. Élimine tout risque de Hard Lockup CPU.
- **Validation Temporelle** : Certification de la réactivité du Sniper et de la précision des filtres Silk/Ghost.
- **Résultat** : Web Server instantané et Bus MSPA 100% protégé.

### 2026-05-11 — Isolation Atomique (v7.5.17-BIBLE-ULTRA)
- **Sécurité Multi-Cœur (Atomics)** : Conversion de toutes les variables partagées (Température, Consigne, États) en `std::atomic<float>` et `std::atomic<bool>`. Élimine les risques de Race Condition observés en v7.5.16.
- **Boot Safety Lock** : Ajout d'un flag `is_ready_` pour différer l'activité du Watchdog (Core 0) tant que le setup n'est pas terminé. Empêche les crashs au démarrage.
- **Sanctuarisation du Core 1** : Confirmation de l'isolation totale. La tâche UART ne fait plus que de la lecture/écriture mémoire atomique.


### 2026-05-11 — Sanctuarisation Architecturale (v7.5.16-BIBLE-ULTRA)
- **Isolation Totale (Core Isolation)** : Découplage radical entre la tâche UART (Core 1) et l'interface utilisateur (Core 0). Suppression de tous les appels `publish_state` du Core 1.
- **Watchdog Sync** : Les mises à jour de l'UI sont désormais gérées exclusivement par le Core 0 à une fréquence stabilisée (2Hz), évitant toute saturation réseau ou CPU.
- **Suivi Moteur Physique** : Ajout du décodage de la trame `0x08` pour distinguer l'activité réelle des pompes de l'affichage des icônes IHM.
- **Résolution du Freeze UI** : En libérant le Core 1 des tâches réseau, le système reste fluide même lors des pires tempêtes de bus ou d'alertes clignotantes.


### 2026-05-11 — Blindage de Production (v7.5.15-PROD-READY)
- **Silk Filter Asymétrique** : Refonte de la logique de lissage (ON immédiat, OFF 1500ms). Garantit une interface stable même lors des alertes clignotantes.
- **Sniper Shield** : La comparaison du Sniper se base désormais sur l'état filtré (`real_`) au lieu de l'état brut du bus, évitant ainsi les tirs parasites pendant les alertes.
- **Silence Radio (UART)** : Passage de tous les logs de synchronisation de bus en niveau `DEBUG`. Supprime le risque de saturation CPU/Logger sur bus bruité.
- **Stabilité Structurelle** : Nettoyage des boucles de suivi pour une réactivité maximale du Core 1.


### 2026-05-11 — Stabilisation du Logger (v7.5.14-STABLE)
- **Throttling des Logs** : Limitation de l'affichage du message `ALERT FILTER ACTIVE!` à une fois toutes les 5 secondes (au lieu de 10 fois par seconde) pour éviter la saturation du processeur et les crashs.
- **Eedomus Guard Opt** : Optimisation de la condition de log pour ne s'activer que si Eedomus est activé.


### 2026-05-11 — Restauration de la Non-Dictature (v7.5.13-STABLE)
- **Conformité MITM** : Suppression de l'armement automatique du Sniper lors de la réception de touches physiques (Keyboard).
- **Principe `target = real`** : Le contrôleur suit désormais les actions physiques au lieu de tenter de les imposer. Résout le conflit de "lutte" (oscillations ON/OFF) lors de l'utilisation des boutons du simulateur ou du spa.


### 2026-05-11 — Résolution Deadlock Mutex (v7.5.12-STABLE)
- **Mutex Récursif** : Migration vers un mutex récursif pour permettre au Sniper d'injecter des commandes pendant que le bus est en cours de lecture par la tâche UART (résolution du deadlock d'auto-blocage).
- **Stabilité Structurelle** : Correction du bug empêchant l'exécution des commandes `Injecting CMD` après un clic dans l'UI.


### 2026-05-11 — Zero-Lag & Eedomus Guard (v7.5.11-STABLE)
- **Eedomus Guard** : Implémentation d'un verrou matériel dans le cœur C++ pour couper toute activité HTTP.
- **Suppression des Latences** : Désactivation totale d'Eedomus pour le LABO, supprimant les blocages de 500ms qui empêchaient le Sniper de fonctionner.
- **Performance** : Gain de fluidité massif sur le traitement du bus UART.


### 2026-05-11 — Diagnostic Commandes (v7.5.10-DEBUG)
- **Log Diagnostic** : Activation de logs INFO sur les changements d'état du bus (`BUS: Filtration -> ON`) et sur les injections Sniper.
- **Audit Sniper** : Suivi des réessais pour comprendre pourquoi certaines commandes "ne partent pas".


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
