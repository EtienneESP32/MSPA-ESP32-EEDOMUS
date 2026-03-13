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

### Test D – Synchro Sniper (Injection sur Poll)
- **Action** : Depuis l'UI ESP32, allumer le chauffage et/ou l'UVC.
- **Objectif** : Vérifier que la commande s'enclenche dans un délai maximal de 9 secondes (attente du Poll 0x0D du clavier).
- **Vérification** : La commande ne doit plus jamais être "ignorée" par le moteur.

### Test E – Exactitude UVC
- **Action** : Appuyer sur l'UVC depuis le clavier physique.
- **Objectif** : Vérifier que l'interface UI de l'ESP32 s'allume bien grâce au nouveau bitmask `0x04` sur la trame `1A`.

### Test F – Simulation erreur (filtre obstrué)
- **Action** : Obstruer légèrement l’aspiration.
- **Objectif** : Voir si erreur F1 ou E1 remonte sur un nouvel ID dédié ou modifie la trame 1A.

## Sniffer UART (analyse protocole, avec ou sans eau)

- **Firmware** : `mspa-uart-sniffer` — pont transparent + décodage des trames (checksum, sens, ID).
- **Logs** : chaque trame valide est loguée en clair (tag `mspa`) : `[SPA->CLAV]` / `[CLAV->SPA]`, hex, nom (Chauffage, Temp, Flags…), et pour IDs inconnus `ID=0x?? DATA=0x??`.
- **Résultats Phase 1 (17/02/2026)** : Flags = **0x1A** uniquement ; UVC = **0x19**. Nouveaux IDs 0x08 (SPA→CLAV), 0x0D (CLAV→SPA).
- **Résultats Phase 2 (05/03/2026, Eau)** : Tests concluants confirmant que `0x1A` porte la totalité de l'état physique (Pompe, Chauffe, Bulles). La trame `0x08` et `0x0D` sont validées comme de simples heartbeats sans données utiles (data `0x00` permanent). La logique du firmware a été corrigée pour n'utiliser que `0x1A`.
