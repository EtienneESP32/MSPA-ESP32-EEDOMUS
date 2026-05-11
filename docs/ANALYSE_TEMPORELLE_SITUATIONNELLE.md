# Analyse Temporelle & Situationnelle - Protocole MSPA

## 1. Contexte Historique : La "Mémoire de l'Eau" (v3.6.2)
La version v3.6.2 (mars 2026) utilisait une règle mathématique de **Rétention (Max Filter)**.

*   **Règle** : `is_ON = (now - last_seen_1_ms < 3000)`
*   **Comportement** : Un bit vu à `1` restait à `1` pendant 3 secondes.
*   **Résultat** : Stabilité absolue de l'UI face au "Blink" (1s ON / 1s OFF). Le "MAX" des deux états était constant.

## 2. Contexte Actuel : La "Patience de la Stabilité" (v7.5.7)
La version v7.5.7 utilise une règle de **Stabilité (Silk Filter)**.

*   **Règle** : `change_state if (value_stable_for > 1500ms)`
*   **Comportement** : On attend que le bus "ne change plus d'avis" pour valider l'état.
*   **Faille Situationnelle** : Si le clignotement du SPA est plus lent (ex: 2s) que le filtre (1,5s), l'ESP valide alternativement le `ON` et le `OFF`.
*   **Conflit Temporel** : Le chauffage et la filtration ne clignotent pas en synchronisation parfaite. L'ESP valide le `OFF` de l'un alors que l'autre est à `ON`, créant l'état physiquement impossible : **"Chauffe ON / Filtre OFF"**.

## 5. La Théorie du Détecteur d'Enveloppe (v3.6.2)
L'analyse de la v3.6.2 révèle que la stabilité n'était pas obtenue par "lissage" mais par **"Verrouillage Temporaire"** (Latching).

*   **Algorithme** : `is_ON = (now - last_seen_high < 3000ms)`
*   **Logique de l'Enveloppe** : Comme un signal radio, on ne regarde pas la fréquence porteuse (le clignotement), mais l'enveloppe globale (l'état souhaité par le SPA).
*   **Découplage** : Cette méthode permet de gérer le pattern `01 -> 02 -> 03` sans jamais afficher de `OFF` parasite, car chaque bit a son propre timer de maintien indépendant.

## 6. Analyse du Dysfonctionnement du SIMU
Le simulateur actuel est "trop parfait". Il lie physiquement la filtration au chauffage. 
*   **Erreur du SIMU** : Il ne produit jamais l'état `02` (Chauffage sans Filtration).
*   **Réalité du SPA** : Le bus UART est un multiplexage visuel. Il peut afficher l'un puis l'autre avec un décalage.
*   **Action** : Le SIMU doit être "dégradé" pour bombarder le bus à 10Hz et désynchroniser les bits.

## 7. Données Techniques Brutes pour Modélisation

### A. Définition des Bits (Registre 0x1A)
*   **Bit 0 (0x01)** : Filtration (Pompe)
*   **Bit 1 (0x02)** : Chauffage (Heat)
*   **Bit 2 (0x04)** : Désinfection (UVC)
*   **Bit 3 (0x08)** : Prêt / Nominal (Steady)

### B. Fréquences & Timings Critiques
*   **Rafraîchissement Bus** : 10 Hz (Trame toutes les 100ms).
*   **Clignotement "Lent" (Alerte/Test)** : 0.5 Hz (1s ON / 1s OFF) ou 0.25 Hz (2s ON / 2s OFF).
*   **Jitter LCD** : Décalage de 50 à 200ms entre l'activation du Bit 0 et du Bit 1.

### C. Le Pattern de l'Opposition (Ghost State)
Transition observée lors de l'activation du chauffage :
1.  `0x01` (Filtre seul)
2.  `0x02` (Chauffe seule - État physiquement faux mais visuellement vrai)
3.  `0x03` (Filtre + Chauffe - État nominal)
4.  `0x01` (Retour au filtre seul pendant la phase de clignotement)

### D. Modèle Mathématique de Reconstruction (Latching)
Pour retrouver la vérité, l'ESP doit appliquer la fonction **MAX_OVER_TIME** :
`Etat_Stable_X = (COUNT_HIGH(Bit_X) > 0 OVER last_5000ms)`

## 8. Stratégie pour le nouveau SIMU (Realistic v1.8)
Pour valider cette logique, le SIMU doit :
1.  Envoyer 20 trames par seconde (Bombardement).
2.  Désynchroniser l'allumage des bits (Filtre à T, Chauffe à T+100ms).
## 9. Dossier de Preuves (Logs de Référence)

### A. Preuve du Pattern "Ghost" (Opposition)
*Source : Logs PROD du 09/05/2026 à 14:28*
```text
14:28:54 [D] Status 0x1A Bits: D1=05  (Filt ON / Chauffe OFF / UVC ON)
14:28:56 [D] 'Filtration': Sending state ON
14:29:00 [D] Status 0x1A Bits: D1=06  (Filt OFF / Chauffe ON / UVC ON)
14:29:02 [D] 'Filtration': Sending state OFF
14:29:02 [D] 'Chauffage': Sending state ON
```
**Analyse** : Le passage de `05` à `06` montre que le bit 0 s'éteint au moment précis où le bit 1 s'allume. C'est la signature du multiplexage visuel.

### B. Référence Historique (Stabilité v3.6.2)
*Fichier source : `archive/firmware/v362.yaml`*
Lignes 186-189 :
```cpp
bool is_f = (id(last_f_ms) != 0 && (now - id(last_f_ms) < 3000));
bool is_h = (id(last_h_ms) != 0 && (now - id(last_h_ms) < 3000));
```
**Justification** : Le timeout de 3000ms permet de "couvrir" les phases de `05` et `06` pour que l'interface eedomus ne reçoive qu'un `ON` permanent pour les deux.

### C. Fichiers d'Archives de Référence
*   `archive/logs/2026-03-21_06_System_Active.txt` : Preuve du pattern `01 -> 02 -> 03` en situation normale.
*   `archive/bin/mspa-sim-v1.8.0-REALISTIC.bin` : Simulateur de test reproduisant ces patterns.
*   `archive/bin/mspa-sim-v1.7.1-ATOMIC.bin` : Simulateur de test "propre" (théorique).

## 10. Résilience Réseau & Gestion des Sockets (v7.5.25)
*Source : Session LABO du 11/05/2026*

### A. Le Paradoxe du Socket (Worst Case Scenario)
Le pire cas d'usage a été modélisé : **Eedomus injoignable + Bombardement de changements d'état.**
*   **Observations** : Une pile TCP/IP peut bloquer la boucle `loop()` pendant ~400ms lors d'un échec de connexion.
*   **Protection** : Le **Cœur 1 (Sanctuaire)** absorbe 100% de la gigue réseau. L'UART reste stable malgré des pauses de 400ms sur le Cœur 0.

### B. Les Trois Piliers de la Survie
1.  **Dédoublonnage (Queue Collapse)** :
    - On ne stocke qu'une seule valeur par périphérique dans la file d'attente.
    - Évite l'empilement de sockets orphelins.
2.  **Socket Guard (Heap Protection)** :
    - Seuil de sécurité : **45 Ko de Heap**.
    - En dessous de ce seuil, l'envoi vers Eedomus est sacrifié pour maintenir l'accès OTA et Web.
3.  **Throttle Dynamique (Gentle Mirror)** :
    - Délai de **5s** entre chaque push.
    - Garantit un flux constant mais jamais brutal.

### C. Validation Technique
*   **Uptime** : Stable (Aucun reboot constaté sous stress).
*   **Heap moyen** : 145 Ko (Récupération immédiate après échec HTTP).
*   **Latence UI** : < 1s (Malgré des timeout de 400ms en arrière-plan).

---
*Sanctuarisé le 11/05/2026 - Version 7.5.25-LABO-RESILIENT*
