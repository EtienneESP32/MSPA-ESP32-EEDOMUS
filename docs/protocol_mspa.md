# Reverse Engineering : Protocole MSPA (Série D)

**Statut** : En cours (phase de capture des commandes)  
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

**Logiques combinatoires** : Chauffage (01) force 02 à 01. UVC (19) s’accompagne de 02 et 15.

## Dictionnaire des états (Spa → Clavier)

| Fonction     | ID (Hex) | Interprétation |
|--------------|----------|----------------|
| Température  | 06       | Valeur / 2 = °C (demi-degrés) |
| Flags        | 1A       | Bitmask : 01=Pompe, 02=Chauffage, 08=Repos ; 03=Pompe+Chauffage |

## Température (demi-degrés)

- **Formule** : `DATA = Température(°C) × 2`
- **Exemple** : 39°C → 78 (0x4E)

## Référence

- Synthèse consolidée 15/02/2026. ID 02 = Filtration (priorité basse), ID 01 = Chauffage (priorité haute).
