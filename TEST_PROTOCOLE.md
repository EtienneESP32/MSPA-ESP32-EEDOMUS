# Protocole de tests – Pont UART SPA / Clavier

Objectif : valider le câblage et le pont **device par device**, puis ensemble.

---

## Rappel du protocole observé

- **Trame clavier → SPA (poll)** : `A5:08:00:AD`
- **Trame SPA → clavier (réponse)** : `A5:06:27:D2`
- Échange cyclique ~400–600 ms.

---

## Phase 1 : Clavier seul (sans SPA)

**But** : Vérifier que le clavier envoie bien des trames et qu’il accepte une réponse « mock » du SPA.

**Câblage** :
- Brancher **uniquement le clavier** sur l’ESP32 (pins 13/14 via level shifter).
- **Ne pas** brancher le SPA (pins 16/17 libres).

**Config** : `test-cool-phase1-clavier-seul.yaml`

**Comportement** :
- L’ESP32 écoute le clavier et affiche les trames en debug (>>> / <<<).
- Dès qu’il reçoit `A5:08:00:AD`, il renvoie `A5:06:27:D2` (simulation SPA).

**Critères de succès** :
- Les logs montrent des `<<< A5:08:00:AD` puis des `>>> A5:06:27:D2`.
- Le clavier **ne** redémarre **pas** et **n’affiche pas** 88.8 en boucle.

**Si échec** : revoir câblage clavier (broches, level shifter, alimentation).

---

## Phase 2 : SPA seul (sans clavier)

**But** : Vérifier que le SPA envoie bien des trames et accepte une réponse « mock » du clavier.

**Câblage** :
- Brancher **uniquement le SPA** sur l’ESP32 (pins 16/17 via level shifter).
- **Ne pas** brancher le clavier (pins 13/14 libres).

**Config** : `test-cool-phase2-spa-seul.yaml`

**Comportement** :
- L’ESP32 écoute le SPA et affiche les trames en debug.
- Dès qu’il reçoit `A5:06:27:D2`, il renvoie `A5:08:00:AD` (simulation clavier).

**Critères de succès** :
- Les logs montrent des `<<< A5:06:27:D2` puis des `>>> A5:08:00:AD`.
- Le SPA reste stable (pas de mode erreur ou arrêt inattendu).

**Si échec** : revoir câblage SPA (broches, level shifter, alimentation).

---

## Phase 3 : Pont complet (clavier + SPA)

**But** : Valider le pont transparent entre les deux appareils.

**Câblage** :
- Clavier sur pins 13/14.
- SPA sur pins 16/17.
- Tous deux via level shifter, alimentation commune.

**Config** : `test-cool.yaml` (pont byte-à-byte, sans mock).

**Critères de succès** :
- Logs avec alternance `<<<` / `>>>` des deux trames.
- Clavier opérationnel (affichage normal, pas de 88.8 ni reboot).
- SPA réagit correctement aux commandes du clavier.

---

## Ordre conseillé

1. **Phase 1** → valider clavier + mock SPA.
2. **Phase 2** → valider SPA + mock clavier.
3. **Phase 3** → flasher le pont complet et tester en conditions réelles.

En cas de problème en phase 3, revenir aux phases 1 et 2 pour confirmer que chaque device fonctionne seul avec l’ESP32.
