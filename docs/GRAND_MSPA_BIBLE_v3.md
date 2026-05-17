# LA GRANDE BIBLE MSPA (v3.7)
## Le Référentiel de Vérité Absolue – Architecture & Protocole

> [!IMPORTANT]
> **Version de Référence : v7.6.0-PLATINUM (Architecture Miroir & Lisseur Physique)**  
> **État du Projet : Certified PLATINUM / Master Sanctuary v5**  
> Ce document intègre le mode miroir UI 100% transparent (sans filtre anti-spam), le lissage domotique Eedomus ultra-réactif (<2s) connecté directement à la trame relais `0x08`, et le filtrage intelligent de l'Alerte Filtre (<800ms).

---

## 1. L'Intelligence du Système (Reverse Engineering)

Le SPA fonctionne selon une architecture découplée où la puissance (Circuit d'eau) et le confort (Circuit d'air) cohabitent sans se connaître.

### A. Cycle de Vie d'une Commande (ex: Chauffage)
1. **INTENTION** : Commande reçue (`0x01`).
2. **ACTION PHYSIQUE** : Le SPA ferme ses relais immédiatement (`0x08` passe à `03`).
3. **TESTS CAPTEURS** : Le SPA vérifie le flux et la température (L'icône `0x1A` clignote).
4. **NOMINAL** : Après ~10s de stabilité, l'icône devient fixe.
5. **SÉCURITÉ (Post-OP)** : À l'arrêt, la pompe continue de tourner ~10s pour refroidir la résistance alors que l'IHM affiche déjà "OFF".

### B. Machine à États du SPA
```mermaid
stateDiagram-v2
    [*] --> IDLE : Mise sous tension
    
    state IDLE {
        direction lr
        note: "Relais 08=00 / Voyant Prêt (1A:08)"
    }

    IDLE --> TESTING : Commande (Filtre/Chauffe/UVC)
    
    state TESTING {
        direction lr
        note: "Relais 08=03 / Voyant CLIGNOTE (1A)"
        [*] --> CheckFlow : Vérification Flux
        CheckFlow --> CheckTemp : Flux OK
        CheckTemp --> [*] : Températ. stable
    }

    TESTING --> RUNNING : Tests réussis (>10s)
    
    state RUNNING {
        direction lr
        note: "Relais 08=03 / Voyant FIXE (1A)"
    }

    RUNNING --> COOLDOWN : Commande OFF (ou Cible atteinte)
    TESTING --> COOLDOWN : Commande OFF
    
    state COOLDOWN {
        direction lr
        note: "Relais 08=03 / Voyant OFF (1A:08)"
        PostCirculation : "Circulation de refroidissement"
    }

    COOLDOWN --> IDLE : Fin tempo (~10s)
    
    state ALARM {
        direction lr
        note: "Erreur (F1, etc.) / Voyants Spécifiques"
    }
    
    ANY --> ALARM : Capteur HS / Surchauffe
```

---

## 2. Le Protocole UART (Le Langage des Bits)

### A. Structure des Trames
*   **Vitesse** : 9600 bauds.
*   **Format** : `[Header] [ID] [Data1] [Data2 (Optionnel)] [Checksum]`
*   **Checksum** : Somme des octets (Header + ID + Data) tronquée à 8 bits.

### B. Dictionnaire des Identifiants (IDs)

| ID | Source | Description | Détails |
| :--- | :--- | :--- | :--- |
| **`0x08`** | SPA | **Relais Physique** | Bit 0: Pompe circulation / Bit 1: Relais chauffe. (100% propre, sans clignotement) |
| **`0x1A`** | SPA | **IHM (Voyants)** | Bit 0: Filtre / Bit 1: Chauffe / Bit 2: UVC / Bit 3: Prêt. |
| **`0x06`** | SPA | **Temp. Eau** | Valeur brute / 2 = Température en °C. |
| **`0x1B`** | SPA | **Consigne & Bulles** | D1: État Bulles / D2: Consigne Température. |
| **`0x04`** | CLAV | **Set Consigne** | Envoi de la nouvelle consigne au SPA. |
| **`0x01`** | CLAV | **Chauffage** | 0x01: ON / 0x00: OFF. |
| **`0x02`** | CLAV | **Filtration** | 0x01: ON / 0x00: OFF. |
| **`0x03`** | CLAV | **Bulles** | 0x01, 0x02, 0x03 (Niveaux) / 0x00: OFF. |
| **`0x19`** | CLAV | **UVC** | 0x01: ON / 0x00: OFF. |

---

## 3. Stratégie de Température "Signed -30"

Pour briser la limite historique des 30°C, nous utilisons la stratégie **`int8_t`**.
*   **Formule** : `Température = 30 + (Valeur_Octet_Signée)`
*   **Plage de fonctionnement** :
    *   `0x0A` (+10) = 40°C
    *   `0x00` ( 0) = 30°C
    *   `0xFF` (-1) = 29°C
    *   `0xFE` (-2) = 28°C
*   **Validation** : Validé sur Contrôleur v6.9.30 et Simulateur v1.7.1.

---

### A. Master Sanctuary v2 (Dual-Core & Atomics)
L'ESP32 est configuré pour garantir que le réseau ne puisse jamais ralentir le bus UART :
*   **Core 1 (uart_task)** : Tâche prioritaire et autonome dédiée à 100% à la gestion de la communication série. Les variables atomiques de l'API standard C++ (`std::atomic`) éliminent tout goulot d'étranglement ou blocage de mémoire (Race Condition).
*   **Core 0 (Main Loop)** : Prend en charge le serveur HTTP, les scripts de push vers l'Eedomus et la gestion WiFi.
*   **Non-Blocking Logic** : Même si le cloud domotique subit d'importantes latences réseau, le bus UART continue de parser les trames physiques sans lag.

---

## 5. L'Architecture PLATINUM (Miroir Transparent & Lisseur Direct)

*   **Couche 1 : Le Miroir Brut 100% Transparent (IHM Locale)** :
    L'IHM locale du contrôleur Web est un miroir transparent de la réalité du bus. En forçant le paramètre de mise à jour à `true` (Option A), nous contournons le filtre anti-spam matériel de 1.5s. L'interface locale clignote en direct et sans décalage au rythme natif (1s ON / 8s OFF en veille, 2s ON / 2s OFF en chauffe), devenant l'outil de diagnostic matériel par excellence.
*   **Couche 2 : Le Lisseur Eedomus Ultra-Réactif sur Relais Physiques (`0x08`)** :
    Le lisseur domotique Eedomus est connecté directement à la vérité des relais matériels de la trame `0x08`. Contrairement aux voyants, les relais sont **propres, fixes et dénués de tout polling**.
    *   **ON** : Instantané dès que le relais physique s'enclenche (<200ms).
    *   **OFF** : Remonté proprement à la domotique en moins de **2 secondes** (contre 30s auparavant) dès l'extinction du moteur, grâce à la réduction du timeout du lisseur, tout en évitant 100% des clignotements.
*   **Couche 3 : Protection d'Alerte Gated** :
    Le filtre anti-parasites d'alerte filtre (`detect_ghost`) utilise un seuil temporel affiné de **800ms**. Cela garantit la capture immédiate d'une alerte réelle (500ms) tout en écartant de façon étanche les pulses de polling (1000ms).

---

## 6. L'Arbitrage Diamond (Phase 4)

*   **L'UI Optimiste** : Pour une sensation de fluidité premium, l'interface bascule sur l'état demandé par l'utilisateur à la milliseconde même du clic (sans attendre le bus).
*   **Le Sniper Déterminé** : Contrairement aux versions précédentes, le Sniper tire systématiquement 10 fois lors d'une action UI, garantissant que même une commande "confuse" (état déjà présumé correct) est envoyée avec fermeté.
*   **Le Watchdog de Réalité (20s)** :
    *   Le système lance un décompte de 20 secondes après chaque commande.
    *   Si à l'issue de ce délai, l'état physique du bus UART ne correspond toujours pas à l'interface, l'ESP force un **Revert**.
    *   L'interface "saute" pour revenir à la réalité physique.
    *   L'Eedomus est immédiatement mise à jour.
*   **Sanctuaire Clavier** : Toute pression sur le clavier physique (vu sur le bus) annule instantanément le Sniper en cours (Règle de Non-Dictature).

> [!IMPORTANT]
> **MAI 2026** : L'Eedomus de PROD autorise désormais les requêtes API en **HTTP standard** (Désactivation du HTTPS obligatoire pour soulager la pile TLS de l'ESP32).

Pour une migration Labo -> Prod sans douleur et sans warnings orange :

### A. La règle du VAR1
Utilisez le champ **`[VAR1]`** du périphérique Eedomus pour stocker l'adresse IP de l'ESP32.
*   **Labo** : `[VAR1] = 192.168.1.226`
*   **Prod** : `[VAR1] = 192.168.1.XXX`

### B. URLs Standardisées (Future-Proof 2026)
L'ESP32 v6.9.30+ impose l'utilisation des Noms d'entités (Majuscules + Espaces) pour éviter les warnings de dépréciation.

*   **Filtration** : `http://[VAR1]/switch/Filtration/turn_on`
*   **Chauffage** : `http://[VAR1]/switch/Chauffage/turn_on`
*   **Consigne [+]** : `http://[VAR1]/switch/Consigne%20Plus/turn_on`
*   **Consigne [-]** : `http://[VAR1]/switch/Consigne%20Moins/turn_on`
*   **Reset Alerte** : `http://[VAR1]/switch/Action%20Reset%20Alerte/turn_on`
*   **Mode Bulles** : `http://[VAR1]/select/Mode%20Bulles/set?option=Niveau1`

---
*Sanctuarisé le 17/05/2026 - Version PLATINUM FINAL - v4.1.*
