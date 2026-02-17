# Plan de tests – Contrôleur MSPA

## Phase 1 : Sans eau (déjà réalisée)

- Validation du pont UART et des IDs (chauffage, filtration, bulles, UVC, consigne).
- Capture des trames et alignement avec `protocol_mspa.md`.

## Phase 2 : Avec eau

### Test A – Flags de charge
- **Action** : Lancer le chauffage avec eau à ~20°C.
- **Objectif** : Vérifier stabilité de l’ID 1A (03 ou autres bits).
- **Log** : Évolution de 1A sur 5 minutes.

### Test B – Calibration température (ID 06)
- **Action** : Comparer ID 06 avec thermomètre externe.
- **Objectif** : Confirmer division par 2, détecter offset éventuel.

### Test C – Fin de cycle
- **Action** : Atteindre la consigne.
- **Objectif** : Observer coupure chauffage (01) et maintien filtration (02) pour refroidir la résistance.

### Test D – Simulation erreur (filtre obstrué)
- **Action** : Obstruer légèrement l’aspiration.
- **Objectif** : Voir si erreur F1 remonte sur un ID dédié ou reste time-out interne clavier.

## Sniffer UART (analyse protocole, avec ou sans eau)

- **Firmware** : `mspa-uart-sniffer` — pont transparent + décodage des trames (checksum, sens, ID).
- **Logs** : chaque trame valide est loguée en clair (tag `mspa`) : `[SPA->CLAV]` / `[CLAV->SPA]`, hex, nom (Chauffage, Temp, Flags…), et pour IDs inconnus `ID=0x?? DATA=0x??`.
- **Résultats (17/02/2026)** : Flags = **0x1A** uniquement ; UVC = **0x19**. Nouveaux IDs 0x08 (SPA→CLAV), 0x0D (CLAV→SPA). Détails dans `TEST_PROTOCOLE.md` et `docs/protocol_mspa.md`.
- Utiliser le sniffer pendant les tests avec eau (Phase 2) pour affiner 0x08 et autres IDs si besoin.
