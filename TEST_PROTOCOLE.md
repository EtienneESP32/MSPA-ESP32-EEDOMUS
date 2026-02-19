# Protocole de tests – Pont UART SPA / Clavier

## État actuel (validé)

Tu as **déjà validé** depuis plusieurs jours :

- **Couche physique** : câblage, level shifter, UART 9600, pins 13/14 (clavier) et 16/17 (SPA).
- **Pont / interception** : l’ESP relaie correctement les trames clavier ↔ SPA (cf. `esphome/mspa-controller.yaml`, `on_loop`).
- **Rejet / dictature UI** : interception des trames clavier, réécriture selon l’état du contrôleur (switches, select), verrouillage (lock) pour bloquer les commandes du clavier physique.

Le firmware en place (`mspa-controller.yaml`) inclut : pont UART, interprétation des IDs (01, 02, 03, 06, 1A, 19), envoi vers eedomus (avec probe de reachabilité et throttle temp), et AP de secours.

---

## Référence : phases physiques (déjà franchies)

Les phases 1 (clavier seul + mock SPA), 2 (SPA seul + mock clavier) et 3 (pont transparent) ont servi à valider le bus et le pont. Les configs de test dédiées ont été retirées du dépôt ; le firmware de production est `esphome/mspa-controller.yaml`, l’audit avec `esphome/mspa-uart-sniffer.yaml`.

---

## Résultats sniffer (17/02/2026) — **au sec** (sans eau)

- **Flags** : états réels (pompe, chauffage, bulles) uniquement sur **0x1A** (pas de 0x00 observé).
- **UVC** : commande/état sur **0x19** (pas de 0x07 observé).
- **Nouveaux IDs** : 0x08 (SPA→CLAV, data 0x00, périodique) ; 0x0D (CLAV→SPA, data 0x00, dans le cycle poll du clavier).
- **Comportement** : le clavier envoie un cycle de commandes (~toutes les 9 s) ; le SPA envoie 0x1A, 0x06 (temp), 0x08 en continu. Les « états réels qui bougent seuls » = trafic normal (SPA + poll clavier).

**À refaire avec eau** (dans quelques jours) : pompe/chauffage/bulles en conditions réelles ; certaines valeurs (0x1A, 0x06, 0x08…) pourront changer et affiner la compréhension.

## Suite : ce qui reste à affiner

- **Avec eau** : confirmer les IDs et flags en conditions réelles (pompe, chauffage, bulles) — voir `docs/test_plan.md` (Phase 2).

Documentation détaillée : `docs/protocol_mspa.md`, `docs/logic_spec.md`, `docs/test_plan.md`.
