# Plan de Tests - Session du 07/05/2026

## 1. Constats d'Analyse (Session de nuit)
*   **Bug de Corruption d'Intention** : La ligne `target.store(active)` dans `update_silk` cause l'arrêt des fonctions si un paquet est perdu (>2.5s). L'intention utilisateur est perdue au profit du "bruit".
*   **Stabilité 0x08** : La Bible (Section 2.B) confirme que l'ID `0x08` est l'état des relais physiques (Bit 0: Pompe, Bit 1: Chauffe). Actuellement, le chauffage ne l'utilise pas.
*   **Identité du Bit Ready** : Confirmation que `0x1A:Raw=0x08` (Bit 3) correspond au voyant "Prêt".
*   **Ghost IDs** : L'ID `0x0B` a été détecté et bloqué avec succès.

## 2. Modifications à implémenter
- [ ] **Sanctuariser `target`** : Modifier `update_silk` pour que `target` ne soit mis à jour par `active` que si `retry == 0` ET que l'état est stable depuis >10s (ou jamais).
- [ ] **Ajouter `physical_h_on_`** : Mapper le Bit 1 de l'ID `0x08` et l'injecter dans le watchdog du chauffage.
- [ ] **Optimiser les Timeouts** : Aligner le timeout de synchronisation sur celui du lien physique (3500ms+) pour éviter les oscillations d'IHM.

## 3. Procédure de Validation
1.  Lancer un cycle de chauffage.
2.  Simuler/Observer des pertes de paquets (via logs).
3.  Vérifier que l'interface Eedomus/Web reste stable même si le bus "scintille".
4.  Vérifier que le `target` reste à `1` tant que l'utilisateur n'a pas cliqué sur OFF.

---
*Document de travail pour la session de tests.*
