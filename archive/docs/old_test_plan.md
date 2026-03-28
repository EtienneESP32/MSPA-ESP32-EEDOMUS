# Plan de tests – Contrôleur MSPA

## Phase 1 : Rétro-ingénierie (2024-2025)
- Validation du pont UART et des IDs de base.
- Capture des trames et alignement avec le dictionnaire d'IDs.

## Phase 2 : Stabilisation MITM (Début 2026)
- **Test Sniper (Poll 0x0D)** : Résultat mitigé sur certaines cartes mères (quelques rejets).
- **Test MITM Shield (0x02)** : Succès total pour empêcher le clavier physique de couper la filtration.

## Phase 3 : Validation Finale (v3.9.3 - Mars 2026)

### Test G – Sniper Synchrone sur 0x06
- **Action** : Lancement Chauffe/UVC/Bulles depuis l'interface HA.
- **Objectif** : Vérifier la réactivité immédiate.
- **Résultat** : **SUCCÈS TOTAL**. L'injection sur la trame 0x06 est acceptée à 100% par le moteur, sans aucune latence ressentie (< 500ms).

### Test H – Sécurité Cross-UI
- **Action** : Couper la Filtration manuellement alors que la Chauffe est ON.
- **Objectif** : Vérifier que l'interface tourne automatiquement la Chauffe sur OFF.
- **Résultat** : **SUCCÈS TOTAL**. La logique d'interlock native d'ESPHome assure la cohérence des icônes.

### Test I – Arrêt Pompe après Chauffe
- **Action** : Arrêter le Chauffage manuellement.
- **Objectif** : Vérifier que la filtration s'éteint automatiquement quelques instants plus tard.
- **Résultat** : **SUCCÈS TOTAL**. Le bouclier MITM se lève dès l'extinction logicielle des organes de charge.

## Conclusion 
Le firmware v3.9.3 est déclaré **STABLE** et constitue la base de référence pour le projet MSPA-ESP32.
