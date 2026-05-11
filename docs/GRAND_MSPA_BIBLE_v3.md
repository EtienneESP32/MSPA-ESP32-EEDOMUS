# LA GRANDE BIBLE MSPA (v3.5)
## Le Référentiel de Vérité Absolue – Architecture & Protocole

> [!IMPORTANT]
> **Version de Référence : v7.5.37-DIAMOND-FINAL**  
> **État du Projet : Certified Diamond / Master Sanctuary v2**  
> Ce document est la synthèse ultime de l'intelligence acquise sur la résilience dual-core et la gestion des flux asynchrones.

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
| **`0x08`** | SPA | **Relais Physique** | Bit 0: Pompe circulation / Bit 1: Relais chauffe. |
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
*   **Core 1 (uart_task)** : Sanctuaire inviolable dédié à 100% au bus série. Utilise des variables **`std::atomic`** pour communiquer ses états sans jamais utiliser de Mutex bloquant dans les chemins critiques.
*   **Core 0 (Main Loop)** : Gère le WiFi, le serveur Web et Eedomus. 
*   **Non-Blocking Logic** : Le Cœur 1 lit et écrit à 10Hz. Si le Cœur 0 est gelé par un timeout réseau, le Cœur 1 continue de piloter le SPA sans aucune gigue.

### B. Gestion des Flux (La Double File FIFO)
Pour une fluidité totale de l'interface Eedomus :
*   **Queue Actions (High Priority)** : Stocke les clics utilisateur. Toujours traitée en premier.
*   **Queue Status (Low Priority)** : Stocke les températures et l'uptime.
*   **Socket Guard** : Suspension automatique de tout envoi vers Eedomus si la Heap descend sous **48 Ko** pour préserver la stabilité du système.
*   **HTTP Fail-Safe** : Libération forcée du verrou réseau après **10 secondes** pour éviter l'asphyxie du Cœur 0.


---

## 5. Logique "Sniper" & "Envelope Detector"

*   **Sniper Shot** : Le contrôleur n'envoie pas de commandes en boucle. Il "tire" 10 à 15 fois avec précision dès qu'un écart est détecté, pour forcer le SPA à obéir.
*   **Règle de Non-Dictature** : Toute action sur le clavier physique ou le simulateur réinitialise instantanément les cibles du Sniper. Le contrôleur suit l'humain au lieu de le combattre.
*   **Envelope Detector (Latching 3s)** : Remplace l'ancien "Silk Filter". Un bit vu à `1` sur le bus reste considéré comme `ON` pendant **3000ms** pour couvrir les phases de clignotement visuel.
*   **Priorité Physique (Règle Hybride)** : En cas de clignotement (Alerte), le système ignore l'enveloppe et affiche l'état réel du moteur (`0x08`). Permet d'afficher "Filtration OFF" pendant une "Alerte Filtre".

---

## 6. Intégration Eedomus Universelle (VAR1)

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
*Sanctuarisé le 12/05/2026 - Version DIAMOND FINAL - v3.5.*
