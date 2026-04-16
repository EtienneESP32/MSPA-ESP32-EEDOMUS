# 💎 MSPA Man-in-the-Middle Ecosystem (ESP32)

[![Project Version: v6.3.7](https://img.shields.io/badge/Controller-v6.3.7--STABLE-blueviolet?style=for-the-badge)](esphome/mspa-controller.yaml)
[![Simulator Version: v1.6.9](https://img.shields.io/badge/Simulator-v1.6.9--ATOMIC-green?style=for-the-badge)](esphome/mspa-simulator.yaml)
[![Sniffer Version: v1.0.0](https://img.shields.io/badge/Sniffer-v1.0.0--STABLE-blue?style=for-the-badge)](esphome/mspa-sniffer.yaml)
[![Home Assistant: Ready](https://img.shields.io/badge/Home%20Assistant-Compatible-orange?style=for-the-badge&logo=home-assistant)](https://www.home-assistant.io/)

> **Keywords**: `MSPA`, `ESP32`, `ESPHome`, `Home Assistant`, `Eedomus`, `Man-in-the-Middle`, `UART`, `Smart-Home`, `DIY-Spa`

Bienvenue dans l'écosystème **MSPA-Controller**, la solution ultime pour transformer ton spa MSPA (Série D) en un objet connecté de pointe, sans sacrifier la sécurité ni la fidélité matérielle.

![Interfaces Labo & Production](assets/images/ui_simulator.png)
*Aperçu du Simulateur (gauche) et du Contrôleur (droite) avec le design Glassmorphism.*

---

## 🏗️ L'Architecture "One-Stop-Shop"

Ce dépôt regroupe les trois piliers essentiels pour domotiser, tester et auditer ton spa :

### 1. 📟 [Le Contrôleur (MITM)](esphome/mspa-controller.yaml)
Le cerveau opérationnel. Placé entre le clavier et le moteur, il agit comme un pont intelligent (**Man-in-the-Middle**).
- **Intégration Eedomus** : Synchronisation bidirectionnelle native (API HTTP).
- **UART Firewall** : Verrouillage du clavier physique à distance.
- **Sniper Engine** : Injection de commandes ultra-précise (latence < 100ms).

![Montage MITM](assets/images/hardware_wiring_main.jpg)
*Le contrôleur ESP32 intercalé entre le clavier et le moteur.*

### 2. 🧪 [Le Simulateur (Banc d'essai)](esphome/mspa-simulator.yaml)
Une copie atomique du spa réel. Indispensable pour développer et tester tes scripts sans sortir au froid.
- **Staggered Heartbeat** : Reproduction millimétrée du flux UART officiel.
- **Atomic Realism** : Signature binaire 100% conforme aux traces matérielles.

### 3. 🔍 [Le Sniffer (Audit)](esphome/mspa-sniffer.yaml)
L'outil de diagnostic passif. Décode et affiche en temps réel les trames `0xA5` circulant sur le bus pour le débogage profond.

---

## 🛡️ Fonctionnalités Premium

```mermaid
sequenceDiagram
    participant C as Clavier Physique
    participant E as ESP32 (Controller)
    participant M as Moteur MSPA
    participant D as Domotique (Eedomus)

    C->>E: "Demande d'allumage (Trames 0x01)"
    alt Firewall Is Locked
        E-->>C: Commande ignorée
    else Firewall Is Open
        E->>M: Commande relayée
    end
    M->>E: Status (Température, État)
    E->>D: Push URL (Mise à jour état)
    D->>E: Commande Cloud
    E->>M: Injection via Sniper Engine
```

### ✨ Pourquoi c'est différent ?
- **Silk Filter** : L'interface utilisateur ne "devine" jamais l'état du spa. Elle attend la confirmation réelle venant du bit de retour de la pompe/chauffe pour s'allumer.
- **Safe-Cap 40°C** : Sécurité logicielle interdisant toute consigne supérieure à la limite constructeur.
---

## 🏠 Compatibilité Home Assistant

Ce projet est **nativement compatible** avec Home Assistant via l'intégration ESPHome.
1.  **Auto-Découverte** : Une fois flashé et sur votre réseau, HA détectera automatiquement le spa.
2.  **Entités** : Toutes les fonctions (Chauffe, Filtre, Bulles, Firewall, Température) remontent comme des entités standard.
3.  **Zéro Config** : Contrairement à d'autres solutions, aucune modification de fichier YAML côté Home Assistant n'est requise.

> [!TIP]
> Si vous utilisez uniquement Home Assistant, vous pouvez passer `eedomus_enabled` à `false` dans votre fichier `secrets.yaml` pour désactiver les requêtes HTTP inutiles vers Eedomus.

### 📱 Interface Eedomus
![Dashboard Eedomus](assets/images/ui_eedomus.png)
*Exemple d'intégration sur le portail Eedomus.*

## 🛠️ Installation & Démarrage

1.  **Matériel** : Consultez la [Liste d'achat validée (BOM)](docs/hardware_bom.md) et le [Schéma de câblage (Synoptique)](docs/hardware_schema.md) pour préparer votre montage.
2.  **Environnement** : Installe [ESPHome](https://esphome.io/) sur ton PC.
3.  **Configuration** :
    - Copie `esphome/secrets.yaml.example` vers `esphome/secrets.yaml`.
    - Remplis tes accès WiFi et tes IDs de périphériques Eedomus.
3.  **Flashage** :
    ```powershell
    # Pour le contrôleur principal
    py -m esphome run esphome/mspa-controller.yaml
    ```

---

## 📸 Conseil Photo & Documentation

> [!IMPORTANT]
> Pour un setup complet, nous recommandons d'ajouter des photos de votre montage dans un dossier `/images` :
> - **Hardware** : Votre câblage Man-in-the-Middle.
> - **Dashboard** : Votre interface Eedomus personnalisée.
> - **WebUI** : L'interface native ESPHome en mode Glassmorphism.

---

## 🆘 Support & Dépannage

Un problème ? Une erreur `4b` ou `5b` dans les logs ?
- Consultez le [**Guide de Dépannage**](docs/troubleshooting.md) pour les solutions aux problèmes courants (Câblage, WiFi, Alimentation).
- Pour toute question technique, n'hésitez pas à ouvrir une **Issue** sur ce dépôt.

---

*Projet maintenu par Etienne. Stabilité validée le 16/04/2026 (Controller v6.3.7 / Simulator v1.6.9).*
