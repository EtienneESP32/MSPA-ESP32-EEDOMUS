# Reverse Engineering : Protocole MSPA (Série D)

**Statut** : Validé par sniffer (17/02/2026) — IDs 0x00/0x1A et 0x07/0x19 tranchés.  
**Matériel** : ESP32 en "Man-in-the-Middle" entre contrôleur (moteur) et clavier.

## Architecture des communications

- **Protocole** : Série TTL 9600 bps, 8N1.
- **Format de trame** : `0xA5` (Start) | ID | Data | Checksum.
- **ESP32** :
  - UART_SPA (moteur) : GPIO16 (RX), GPIO17 (TX)
  - UART_CLAVIER : GPIO14 (RX), GPIO13 (TX)

## Structure d’une trame (4 octets)

| Octet | Nom      | Description                          |
|-------|----------|--------------------------------------|
| 0     | Header   | Toujours `0xA5`                      |
| 1     | ID       | Identifie la fonction               |
| 2     | DATA     | Valeur (état 0/1, temp×2, etc.)     |
| 3     | Checksum | (Octet0 + Octet1 + Octet2) & 0xFF   |

## Dictionnaire des IDs (Clavier → Spa)

| Fonction   | ID (Hex) | Data (Hex)     | Exemple      |
|------------|----------|-----------------|--------------|
| Chauffage  | 01       | 01 / 00         | A5 01 01 A7  |
| Filtration | 02       | 01 / 00         | A5 02 01 A8  |
| Bulles     | 03       | 00 à 03         | A5 03 01 A9  |
| Consigne   | 04       | Temp×2          | A5 04 4E F7  |
| UVC        | 19       | 01 / 00         | A5 19 01 BF  |
| Sanitize   | 15       | 01              | A5 15 01 BB  |
| (poll/touche) | 0D    | 00              | A5 0D 00 B2  |

**Logiques combinatoires** : Chauffage (01) force 02 à 01. UVC (19) s’accompagne de 02 et 15.

## Dictionnaire des états (Spa → Clavier)

| Fonction     | ID (Hex) | Interprétation |
|--------------|----------|----------------|
| Température  | 06       | Valeur / 2 = °C (demi-degrés) |
| Flags        | 1A       | Bitmask : 01=Pompe, 02=Chauffage, 04=Bulles. **En pratique** le SPA envoie 0x1A en alternance (data 0x03 = pompe+chauffage, 0x04 = bulles, 0x00 = tout off) ; le contrôleur fusionne ces trames pour afficher un état unique. |
| (inconnu)    | 08       | SPA→CLAV, data 0x00 observé en continu — rôle à préciser |

## Température (demi-degrés)

- **Formule** : `DATA = Température(°C) × 2`
- **Exemple** : 39°C → 78 (0x4E)

## Référence

- Synthèse consolidée 15/02/2026. ID 02 = Filtration (priorité basse), ID 01 = Chauffage (priorité haute).
- **Sniffer 17/02/2026 (au sec)** : UVC confirmé **0x19** (pas 0x07). Flags confirmés **0x1A** (pas 0x00). Nouveaux IDs 0x08 (SPA→CLAV), 0x0D (CLAV→SPA, cycle poll clavier). Session avec eau à prévoir pour affiner.
