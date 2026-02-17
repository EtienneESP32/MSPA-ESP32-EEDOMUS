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

## Sniffer UART

- Utiliser le firmware `mspa-uart-sniffer` pour capturer toutes les trames pendant les tests avec eau (pompe en route) et compléter le dictionnaire des IDs si nécessaire.
