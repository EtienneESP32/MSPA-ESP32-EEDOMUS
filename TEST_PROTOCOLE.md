# Protocole de tests – Pont UART SPA / Clavier

## État actuel (validé)

Tu as **déjà validé** depuis plusieurs jours :

- **Couche physique** : câblage, level shifter, UART 9600, pins 13/14 (clavier) et 16/17 (SPA).
- **Pont / interception** : l’ESP relaie correctement les trames clavier ↔ SPA (cf. `esphome/mspa-controller.yaml`, `on_loop`).
- **Rejet / dictature UI** : interception des trames clavier, réécriture selon l’état du contrôleur (switches, select), verrouillage (lock) pour bloquer les commandes du clavier physique.

Le firmware en place (`mspa-controller.yaml`) inclut : pont UART, interprétation des IDs (01, 02, 03, 06, 1A, 19), envoi vers eedomus (avec probe de reachabilité et throttle temp), et AP de secours.

---

## Référence : phases physiques (déjà franchies)

Les étapes ci‑dessous ont servi à valider le bus et le pont. Elles restent en référence si tu dois re-tester un device isolé.

| Phase | Objectif | Config (référence) | Statut |
|-------|----------|--------------------|--------|
| 1 | Clavier seul + mock SPA | `test-cool-phase1-clavier-seul.yaml` | Validé |
| 2 | SPA seul + mock clavier | `test-cool-phase2-spa-seul.yaml` | Validé |
| 3 | Pont complet transparent | `test-cool.yaml` | Validé |

---

## Suite : ce qui reste à affiner

- **Avec eau** : confirmer les IDs et flags en conditions réelles (pompe, chauffage, bulles) — voir `docs/test_plan.md` (Phase 2).
- **Sniffer / catch** : utiliser `mspa-uart-sniffer.yaml` ou le firmware « catch » pour enregistrer les trames avec eau et compléter `docs/protocol_mspa.md` si de nouveaux IDs apparaissent (ex. 0x00 vs 0x1A pour les flags, 0x07 vs 0x19 pour l’UVC).

Documentation détaillée : `docs/protocol_mspa.md`, `docs/logic_spec.md`, `docs/test_plan.md`.
