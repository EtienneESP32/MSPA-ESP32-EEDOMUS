# Reverse Engineering : Protocole MSPA (D-Series)

**Statut** : Validé par sniffer — IDs 0x01, 0x02, 0x03, 0x06, 0x1A, 0x19, 0x1B tranchés.
**Matériel** : ESP32 standard (Man-in-the-Middle) entre moteur et clavier.

## Architecture des communications

- **Protocole** : Série TTL 9600 bps, 8N1.
- **ESP32** :
  - UART_SPA (moteur) : GPIO16 (RX), GPIO17 (TX)
  - UART_CLAVIER : GPIO14 (RX), GPIO13 (TX)

## Structure des trames (4 et 5 octets)

### Trame Standard (4 octets)
`0xA5` | ID | Data | Checksum
- **Calcul** : `Check = (B0 + B1 + B2) & 0xFF` (Somme simple)
- Utilisée pour les commandes de base et la température standard.

### Trame Étendue (5 octets)
- **ID 0x1B** : Retour d'état étendu (`00 1B D1 D2 CHK`).
  - **D1** : Niveau des Bulles (0: OFF, 1-3: Niveaux).
  - **D2** : Température de Consigne. Formule : `D2 = Consigne - 30`. (Ex: 10 = 40°C).
- **Calcul** : `Check = (B1 + B2 + B3 - 0x11) & 0xFF` (Somme des données moins 17).

---

## Dictionnaire des IDs

| Fonction | ID (Hex) | Description |
|------------|----------|-------------|
| Chauffage  | 01       | Commande ON (01) / OFF (00) |
| Filtration | 02       | Commande ON (01) / OFF (00) |
| Bulles     | 03       | Commande Niveau 1 à 3 / 0 (OFF) |
| Consigne   | 04       | Température x 2 |
| Poll Temp  | 06       | Température Eau (Moteur -> Clavier). **Fenêtre de Sniper stable.** |
| Heartbeat  | 08       | **Vérité Physique (Relais)**. Stable. `00`: Repos, `03`: Actif (Pompe/Chauffe). |
| Presence   | 12       | **Heartbeat Bulles**. `01` si le module est présent/actif. |
| Poll/Ping  | 0D       | Clavier -> SPA. (Ancien point de sniper). |

### États Moteur (Spa → Clavier)
- **ID 0x1A** : **Vérité Visuelle (Interface)**. Bit 0: Pompe, Bit 1: Chauffe, Bit 2: UVC, Bit 3: Prêt. Clignote pendant les phases de test ou erreurs.
- **ID 0x1B** : **Extended Status**. D1 = Bulles, D2 = Consigne.

---

## Historique des Découvertes

### 22/03/2026 : v3.9.3 (Validation Finale)
- **Déclencheur Sniper** : L'ID `0x06` (SPA -> Clavier) est identifié comme le point d'injection idéal pour garantir l'acceptation des commandes par la carte mère.
### 23/03/2026 : Le Breakthrough du Dual Truth (v3.9.3+)
- **Réhabilitation du 0x08** : Contrairement aux idées reçues, l'ID `0x08` n'est pas vide. Il contient la **vérité binaire des relais** (`03` = ON, `00` = OFF). C'est le signal le plus stable du protocole car il ne clignote jamais.
- **Définition du 1A** : L'ID `1A` est reclassé comme "Trame de Dialogue Visuel". Il sert à commander l'affichage du clavier, d'où ses clignotements natifs (1Hz) durant les phases de démarrage.
- **Identification du 0x12** : Confirmé comme étant le signal de présence/activité du module Bulles.
