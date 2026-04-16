# Architecture du Banc de Test (Simulateur MSPA)

L'utilisation d'un second ESP32 pour simuler le SPA est la **meilleure garantie de sécurité** possible. Cela permet de valider le Firewall v6.x avant toute mise en production.

## 1. Schéma de Câblage (Banc de Test)

| ESP32 Contrôleur (v6.3.7) | ESP32 Simulateur (v1.6.9) | Rôle |
| :--- | :--- | :--- |
| **GPIO16 (RX_SPA)** | **GPIO17 (TX_MB)** | Simulation Carte Mère |
| **GPIO17 (TX_SPA)** | **GPIO16 (RX_MB)** | Réception ordres vers MB |
| **GPIO14 (RX_KBD)** | **GPIO13 (TX_KBD)** | Simulation Clavier |
| **GPIO13 (TX_KBD)** | **GPIO14 (RX_KBD)** | Réception ordres vers Clavier |
| **GND** | **GND** | Masse commune (Obligatoire) |

## 2. Logique du Simulateur (v1.6.9 ATOMIC)

Le simulateur reproduit fidèlement le comportement temporel du SPA :
1.  **Boucle Carte Mère** : Envoie `A5 06`, `A5 08`, `A5 1A`, `A5 1B`, `A5 12` toutes les 100ms.
2.  **Boucle Clavier** : Envoie `A5 0D` (Heartbeat) toutes les 100ms.
3.  **Réaction** : Si le simulateur reçoit `A5 01 01` (Cmd Chauffe), il déclenche ses scripts internes (Phase 1, Phase 2) et modifie les trames de retour.

## 3. Protocole de Validation v6.3.7 (Firewall & Reset)

Avant déploiement sur le SPA réel, le banc valide :
1.  **Vérification Firewall** : Activer `sw_lock` sur le contrôleur. Actionner les boutons "ACTION" sur le simulateur. Vérifier que les commandes sont **interceptées et loguées** sans atteindre le bus principal.
2.  **Stress Test UART** : Vérifier la stabilité du décodage 5-octets (`1B`) sous charge.
3.  **Surveillance OTA** : Vérifier que les mises à jour ne sont pas impactées par le trafic UART.
