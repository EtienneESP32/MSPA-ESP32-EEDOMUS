# 🔌 Bill of Materials (BOM) - Matériel MSPA

Ce document répertorie les composants matériels **testés et validés** pour le projet MSPA (Contrôleur, Simulateur et Sniffer). L'utilisation de ces composants garantit une compatibilité maximale avec le code ESPHome fourni.

---

## 🛠️ Liste des Composants

| Composant | Rôle technique | Lien d'achat |
| :--- | :--- | :--- |
| **ESP32 DevKit** | Le cerveau du projet (WiFi, BLE, UART). Format compact. | [Acheter sur Amazon](https://www.amazon.fr/dp/B0BWN595WY) |
| **Boîtier Projet DIY** | Protection de l'électronique contre l'humidité (Espace suffisant pour ESP32 + convertisseurs). | [Acheter sur Amazon](https://www.amazon.fr/dp/B0824HTB4F) |
| **Connecteurs JST SM** | Connecteurs 4 broches identiques à ceux d'origine du spa (Pas besoin de couper les fils). | [Acheter sur Amazon](https://www.amazon.fr/dp/B0CBWX98NF) |
| **Level Shifters (3.3V/5V)** | **CRUCIAL** : Adaptateur de tension pour protéger l'ESP32 (3.3V) des signaux du Spa (5V). | [Acheter sur Amazon](https://www.amazon.fr/dp/B0CJ6PD2GR) |

---

## 💡 Détails Techniques

### 0. Identification (Carte Mère SPA)
![Carte Mère SPA](assets/images/mspa_motherboard.jpg)
*L'intérieur du boîtier de contrôle du spa MSPA.*

### 1. Pourquoi des Level Shifters ?
![Module de Conversion](assets/images/hardware_level_shifter.jpg)
*Le convertisseur bidirectionnel servant de barrière de tension.*

Le bus de données du spa MSPA fonctionne en **5V Logic**. L'ESP32, lui, fonctionne en **3.3V Logic**. 
> [!WARNING]
> Connecter l'UART du spa directement sur l'ESP32 sans ces convertisseurs peut **détruire votre microcontrôleur** ou causer des erreurs de communication. Les modules bidirectionnels à 4 canaux recommandés ici sont parfaits pour gérer les deux UART (TX/RX) en toute sécurité.

### 2. Le choix du boîtier
Le boîtier YoiYee (95x55x23mm) est compact mais permet de loger confortablement l'ESP32 et les modules de conversion. Il est suffisant pour être placé à l'intérieur du bloc moteur ou dans une boîte de dérivation externe.

![Montage Propre](assets/images/hardware_esp_level_shifter.jpg)
*Exemple de montage : le convertisseur est soudé directement sur l'ESP32 pour économiser de la place.*

### 3. Connectique sans soudure (Propre)
![Connecteurs JST-SM](assets/images/hardware_connector_jst.jpg)
*Connecteurs standard 4 broches permettant une installation MITM "Plug-and-Play".*
En utilisant les câbles JST SM 4-Pin, vous pouvez créer une "rallonge" ou une dérivation (T-Tap) qui se branche directement sur les prises d'origine du spa. Cela permet de retirer le système domotique sans laisser de trace si nécessaire.

---

*Dernière mise à jour : 04/04/2026. Matériel validé pour firmware v6.3.6.*
