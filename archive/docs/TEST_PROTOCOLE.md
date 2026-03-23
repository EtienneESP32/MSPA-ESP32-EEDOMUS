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

### Phase 3 : Injection Absolue (v3.2.1)
- Validation des IDs `0x01` (Chauffe) et `0x02` (Pompe).
- Stratégie validée : Injection "One-Shot" lors de la détection de l'ID `0x0D` (Poll).

### Phase 4 : Interception Sélective (v3.4.11)
- **Breakthrough** : Identification formelle des trames de 5 octets (`Header 00`) pour les bulles.
- **Checksum** : Découverte de la formule `Somme - 0x11` pour les trames étendues.
- **MITM de Précision** : Abandon de l'injection par dessus le clavier. Passage à l'interception et réécriture des trames de 4 octets (`OFF` -> `ON`) dans le buffer UART.
- **Résultat attendu** : Zéro conflit clavier/ESP, retour d'état bulles fonctionnel, température stable.

## Résultats sniffer (05/03/2026) — **en eau**

- **Absolute Sync (Injection de commandes)** : Confirmée ! Fonctionnement parfait. L'ESP intercale l'envoi d'une commande unique et l'état des flags `0x1A` s'adapte en conséquence.
- **Flags Moteur** : Confirmation définitive que c'est sur l'ID **`0x1A`** (passe de `0x03` pompe+chauffe, à `0x04` ou `0x08` lors des coupures).
- **Bruit/Heartbeat** : La trame **`0x08`** suspectée auparavant de porter les flags est définitivement qualifiée de pur signal Heartbeat (sa data est immuablement à `0x00`).
- **Correction** : Le firmware principal (`mspa-controller.yaml`) a été patché v3.2.1 pour ne s'appuyer que sur `0x1A`.

## Suite : Finalisation logicielle

- L'ensemble du protocole MSPA D-Series est considéré comme totalement décodé et l'architecture *One-Shot Injection / Absolute Pass-Through* pleinement validée avec l'eau et les vraies charges de puissance.
- Documentation mise à jour : `docs/protocol_mspa.md`, `docs/test_plan.md`.
