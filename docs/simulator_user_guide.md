# Guide d'utilisation : Simulateur MSPA (v1.6.9 ATOMIC)

Ce document résume le fonctionnement du banc de test matériel pour valider le système MSPA.

## 1. Interface de Simulation (v1.6.9)

L'interface est structurée pour séparer l'ordre (Simulation) du retour réel (Feedback Bus).

| Type | Nom | Fonction |
| :--- | :--- | :--- |
| **Input (Droite)** | `SIM: XXX` | **La Simulation**. Valeur injectée sur le bus. |
| **Output (Gauche)** | `LUE: XXX (Bus)` | **Le Feedback**. Ce que le contrôleur décode réellement. |

## 2. Alerte & Reset Changement de Filtre (v1.6.9)

Le simulateur gère nativement le cycle d'alerte de filtre :
- **Déclenchement** : Appuyer sur le bouton `ALERTE FILTRE F1` pour activer l'alerte (le bit 0x1A clignote, la pompe s'arrête).
- **Reset** : Depuis le **contrôleur**, activer le switch "Action Reset Alerte". Le contrôleur envoie une rafale de trames `0x02 0x01` pendant 4 secondes.
- **Détection** : Le simulateur valide le Reset après **3 secondes continues** de réception sans interruption (>500ms entre trames). L'alerte est alors effacée.

> ⚠️ Note : Pendant la détection du Reset, la barrière anti-vibration (500ms) ne s'applique **pas** à la commande 0x02 pour permettre la capture haute fréquence (10Hz).

## 3. Gestion des Bulles (Cycle Natif)
Le simulateur implémente désormais le cycle réel des spas D-Series :
- **Ordre de cycle** : `0 (Arrêt) -> 2 (Moyen) -> 3 (Fort) -> 1 (Doux) -> 0`.
- Toute commande `0x03` reçue sur l'UART déclenche le passage au niveau suivant dans cet ordre précis.

## 3. Branchement du Banc de Test

| ESP32-Sim (Le SPA) | ESP32-Contrôleur (Le Cerveau) |
| :--- | :--- |
| **GND** | **GND** |
| **GPIO17 (TX1)** | **GPIO16 (RX_SPA)** |
| **GPIO16 (RX1)** | **GPIO17 (TX_SPA)** |
| **GPIO13 (TX2)** | **GPIO14 (RX_KBD)** |
| **GPIO14 (RX2)** | **GPIO13 (TX_KBD)** |

## 5. Maintenance & Signaux
- **LED Physique (GPIO 2)** :
  - **Flash rapide** : Activité UART (Trames circulantes).
  - **Allumage 1s** : Commande valide (`A5`) traitée par le simulateur.
- **Diagnostics** : Les compteurs `DIAG: Erreurs 4b/5b` permettent de vérifier l'intégrité du bus sous stress.
- **Limite Thermique** : Le simulateur accepte des valeurs jusqu'à 50°C, mais le contrôleur v6.x bridera tout affichage/consigne à 40°C par sécurité.
