# Guide d'utilisation : Simulateur MSPA (v1.5.8 STABLE)

Ce document résume le fonctionnement de ton banc de test matériel pour valider la transition v4.0.

## 1. Nouvelle Interface Découplée (Exclu v1.5.8)

L'interface a été restructurée pour éviter les conflits de rafraîchissement (race conditions) entre tes ordres et le retour du bus UART. Chaque ligne de température (`Eau` et `Consigne`) dispose désormais de deux rôles distincts :

| Type | Nom | Fonction |
| :--- | :--- | :--- |
| **Input (Droite)** | `SIM: XXX` | **La Simulation**. C'est ici que tu règles la valeur que tu souhaites injecter sur le bus. Ces champs ne bougent pas tout seuls. |
| **Output (Gauche)** | `LUE: XXX (Bus)` | **Le Feedback**. Ce senseur affiche la valeur réellement décodée sur les fils. Si tu changes la simulation, ce chiffre doit suivre instantanément. |

> [!TIP]
> Si `LUE` ne suit pas `SIM`, c'est le signe d'une erreur de protocole, d'un mauvais checksum ou d'une déconnexion du bus UART.

## 2. Branchement du Banc de Test (ESP32-WROOM)

| ESP32-Sim (Le SPA) | ESP32-Contrôleur (Le Cerveau) |
| :--- | :--- |
| **GND** | **GND** (Indispensable) |
| **GPIO17 (TX1)** | **GPIO16 (RX_SPA)** |
| **GPIO16 (RX1)** | **GPIO17 (TX_SPA)** |
| **GPIO13 (TX2)** | **GPIO14 (RX_KBD)** |
| **GPIO14 (RX2)** | **GPIO13 (TX_KBD)** |

## 3. Scénario de Test v1.5.8

1. **Vérification du Boot** : Au démarrage, `LUE: Temp Eau` doit afficher **10.0°C**.
2. **Simulation Thermique** :
   - Passe `SIM: Temp Eau` à **38.0°C**.
   - Vérifie que `LUE: Temp Eau (Bus)` passe à **38.0°C**.
3. **Simulation Consigne** :
   - Passe `SIM: Consigne` à **40°C**.
   - Vérifie que `LUE: Consigne (Bus)` passe à **40°C**.

## 4. Maintenance de l'OTA
- **IP Fixe** : `192.168.1.230`
- **LED Bleue (GPIO 2)** :
  - Clignotement rapide = Activité UART détectée.
  - Allumage Fixe = Commande valide reçue et traitée.
