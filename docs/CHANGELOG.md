# Historique des changements (MSPA ESP32)

### 2026-04-04 – Alerte eedomus & Master Sync (v6.3.6-STABLE) – ACTUEL
- **eedomus** : Implémentation de la remontée d'alerte automatique du filtre (ID 3536562).
- **Welcome Sync** : Script de synchronisation exhaustive (8 entités) au démarrage avec délai de 500ms pour éviter la surcharge.
- **Sécurité** : Point d'accès `MSPA-RECOVERY` désormais protégé par mot de passe (via secrets).
- **Logique d'état** : Harmonisation des noms d'entités pour une compatibilité 100% sans changer les réglages eedomus.
- *Projet maintenu par Etienne - Stabilité Master validée le 04/04/2026 (Sync & Security OK). Version 6.3.6-STABLE.*

### 2026-03-28 – Audit & Diagnostic (v6.3.5-STABLE)
- **API Native ESPHome** : Activation du port API (6053) avec `reboot_timeout: 0s`. Permet le monitoring et les logs à distance sans risque de redémarrage cyclique.

### 2026-03-28 – Release Finale (v6.3.4-STABLE)
- **UART Firewall (Production)** : Filtrage sélectif actif. Blocage des commandes physiques en mode "Verrouillage". Heartbeat `0x0D` préservé.
- **Miroir d'état (100% Natif)** : L'interface suit strictement le bus UART, pas de forcage fictif.
- **Auto-Correction Sniper v2** : Ré-essais automatiques en cas de perte de trame.
- **Alerte Filtre (DIAG)** : Détection intelligente du clignotement filtre via `0x1A/0x08`.

### 2026-03-27 – Simulateur Labo (v1.6.0-STABLE)
- **Cycle des Bulles Natif** : Implémentation du cycle `0 -> 2 -> 3 -> 1 -> 0`.
- **Zéro Backdoor** : Suppression des accès directs à la mémoire. Tout passe par le bus UART pour une simulation 100% fidèle.
- **Bridge UART Dédié** : Support complet des trames 5-octets (`0x1B`).

### 2026-03-22 – UI Intelligente (v3.9.3) – ANCIENNE BASE STABLE
- **Sécurités d'Extinction (Interlocks)** : Cascade Filtration/Chauffe/UVC gérée par ESPHome.
