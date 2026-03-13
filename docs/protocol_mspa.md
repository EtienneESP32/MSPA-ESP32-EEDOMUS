# Reverse Engineering : Protocole MSPA (D-Series)

**Statut** : Validé par sniffer — IDs 0x01, 0x02, 0x03, 0x06, 0x1A, 0x19, 01B tranchés.
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
  - **D1** : Niveau des Bulles (0: OFF, 1: Lvl 1, 2: Lvl 2, 3: Lvl 3).
  - **D2** : Température de Consigne. Formule : `D2 = Consigne - 30`. (Ex: 10 = 40°C).
- **Calcul** : `Check = (B1 + B2 + B3 - 0x11) & 0xFF` (Somme des données moins 17).
- *Note* : La trame `00 1B 01 0A 15` correspond donc à "Bulles Niveau 1, Consigne 40°C".

---

## Dictionnaire des IDs

| Fonction | ID (Hex) | Data Relâché | Data Pressé | Structure |
|------------|----------|--------------|----------------|-----------|
| Chauffage  | 01       | 00           | 01             | 4 octets  |
| Filtration | 02       | 00           | 01             | 4 octets  |
| Bulles     | 03       | 00           | 01 à 03 (Lvl)  | 4 octets  |
| Consigne   | 04       | Temp×2       | Temp×2         | 4 octets  |
| UVC        | 19       | 00           | 01             | 4 octets  |
| Poll/Sync  | 0D       | 00           | N/A            | 4 octets  |

### États Moteur (Spa → Clavier)
- **ID 0x1A** : Flags réels (4 octets). Bit 0: Pompe, Bit 1: Chauffe, Bit 2: UVC/Ozonateur, Bit 3: Prêt.
- **ID 0x1B** : Extended Status (5 octets). `00 1B D1 D2 CHK`.
  - **D1** : Niveau Bulles (0-3). Source de vérité UNIQUE.
  - **D2** : Température de Consigne (`Consigne - 30`).
- **ID 0x06** : Température Eau (4 ou 5 octets).

---

## Historique des Découvertes

### 05/03/2026 : Validation "en eau"
- Confirmation définitive de l'ID `0x1A` comme source de vérité des flags.

### 08/03/2026 : Analyse v3.4.15 & Conflit OTA
- **Bug Diagnostiqué** : L'ID `0x1B` (commençant par `0x00`) est interceptée par la couche OTA d'ESPHome (`Magic bytes mismatch`).
- **Conflit d'État** : L'octet `D2` de la trame `1B` (Consigne) entrait en conflit avec les flags `1A` (Pompe/Chauffe) via une variable globale partagée.
- **Stabilisation (v3.4.16)** : Découplage complet des variables et contournement du bruit OTA.

### 11/03/2026 : Injection Synchrone et Bitmask UVC (v3.5.0)
- **Bug Diagnostiqué** : Les commandes ESP de Chauffage (0x01) et UVC (0x19) étaient aléatoirement rejetées par la carte mère.
- **Explication** : La carte mère n'écoute l'UART qu'immédiatement après avoir reçu un `Poll` (`0x0D`) du clavier (envoyé toutes les 9 secondes). 
- **Bitmask UVC** : L'état naturel de l'UVC/Ozonateur a été confirmé sur le **Bit 2 (`0x04`)** de la trame `0x1A`.
