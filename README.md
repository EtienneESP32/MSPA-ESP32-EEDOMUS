# Contrôleur MSPA (ESP32)

Pont UART **man-in-the-middle** entre le clavier et le moteur d’un spa MSPA (série D), avec pilotage domotique (eedomus), interface Web et **Firewall UART** intégré.

## 🚀 État du Projet (v6.3.6-STABLE)

Le projet est actuellement dans sa version la plus stable et aboutie, validée sur banc de test et en situation réelle.

*   **Firmware Contrôleur** : [v6.3.6-STABLE](file:///c:/Users/ebesa/Documents/MSPA/esphome/mspa-controller.yaml)
*   **Simulateur Labo** : [v1.6.0-STABLE](file:///c:/Users/ebesa/Documents/MSPA/esphome/mspa-simulator.yaml)

---

## 🗺️ Cartographie du Dépôt

Pour comprendre le projet en une passe, voici comment les fichiers sont organisés :

### 1. Cœur du Système (Dossier `/esphome`)
C'est ici que se trouve le code "vivant" qui tourne sur tes ESP32.
- **[mspa-controller.yaml](file:///c:/Users/ebesa/Documents/MSPA/esphome/mspa-controller.yaml)** : Le micrologiciel principal. Il gère le WiFi, l'interface Web, et la communication avec Eedomus.
- **[components/mspa_uart/](file:///c:/Users/ebesa/Documents/MSPA/esphome/components/mspa_uart/)** : Le moteur C++ (`mspa_uart.h`). C'est le cerveau qui décode le protocole MSPA, gère le **Firewall** et le **Sniper Engine**.
- **[mspa-simulator.yaml](file:///c:/Users/ebesa/Documents/MSPA/esphome/mspa-simulator.yaml)** : Le micrologiciel du SPA virtuel pour tes tests sur banc.

### 2. Documentation de Référence (Dossier `/docs`)
Les guides essentiels pour la maintenance et l'utilisation quotidienne.
- **[mspa_full_reverse.md](file:///c:/Users/ebesa/Documents/MSPA/docs/mspa_full_reverse.md)** : La "Bible" du protocole. Tout ce qu'on sait sur les trames `0xA5`.
- **[test_bench_simulator.md](file:///c:/Users/ebesa/Documents/MSPA/docs/test_bench_simulator.md)** : Comment câbler et utiliser ton banc de test.
- **[simulator_user_guide.md](file:///c:/Users/ebesa/Documents/MSPA/docs/simulator_user_guide.md)** : Guide précis des fonctions du simulateur v1.6.0.
- **[config_eedomus.md](file:///c:/Users/ebesa/Documents/MSPA/docs/config_eedomus.md)** : Guide d'installation des périphériques sur ton portail Eedomus.
- **[CHANGELOG.md](file:///c:/Users/ebesa/Documents/MSPA/docs/CHANGELOG.md)** : Historique des versions et des corrections majeures.

### 3. Mémoire du Projet (Dossier `/archive`)
L'historique des recherches, des anciens prototypes et des études de risques passées.
- **[v4.0_alpha/](file:///c:/Users/ebesa/Documents/MSPA/archive/firmware/v4.0_alpha/)** : Anciens prototypes du firewall.
- **[docs/](file:///c:/Users/ebesa/Documents/MSPA/archive/docs/)** : Analyses de risques, anciens schémas et études comparatives de projets similaires.

---

## 🛠️ Installation Rapide

1.  **Secrets** : Copie `esphome/secrets.yaml.example` vers `esphome/secrets.yaml` et remplis tes accès WiFi/API.
2.  **Flash (USB)** :
    ```bash
    py -m esphome run esphome/mspa-controller.yaml --device COM3
    ```
3.  **Flash (OTA)** : Une fois sur le réseau, utilise l'IP fixe définie dans tes secrets.

---

## ✨ Fonctionnalités Clés (Architecture v6.x)

- **UART Firewall** : Le clavier physique peut être verrouillé à distance (`sw_lock`) pour empêcher toute manipulation locale, tout en préservant le lien vital (Heartbeat) avec le moteur.
- **Silk Filter (Miroir)** : L'interface ne "devine" jamais l'état du spa. Elle attend la confirmation réelle venant du bus UART pour mettre à jour les boutons.
- **Sniper Engine** : Les commandes sont injectées avec une précision millimétrée après chaque trame de température pour garantir un taux de succès de 100%.
- **Safe-Cap 40°C** : Sécurité logicielle interdisant toute consigne supérieure à 40°C.

---
*Projet maintenu par Etienne - Stabilité Master validée le 04/04/2026. Version 6.3.6-STABLE.*
