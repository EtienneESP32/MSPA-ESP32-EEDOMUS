# MSPA Universal Bible (v2.2)
## Référentiel de Vérité Absolue - Logiciel & Protocole

> [!IMPORTANT]
> **Version de Référence : 6.9.30-PROD (Network Optimized)**
> Cette version parachève la stabilité réseau en isolant totalement les communications série (UART) des requêtes HTTP.

---

### 1. Protocole de Communication (Stratégie Température)
Le bus UART (9600 bauds) utilise deux stratégies distinctes :
*   **Température de l'eau (Sonde 0x06)** : Stratégie `* 2`.
*   **Consigne de Température (ID 0x04 & 0x1B)** : Stratégie `- 30`.
*   **Règle d'or du Signe** : L'octet de consigne est un **`int8_t` (signé)**. Permet de descendre sous les 30°C (`0xFE` = 28°C, `0xFF` = 29°C).

---

### 2. Le "Silk Filter" (Fenêtre de 1500ms)
La logique "Clockwork" filtre la réalité du bus :
*   **Lissage** : Évite les sauts d'IHM lors des clignotements d'alerte.
*   **Alerte** : Détectée si `Signal_Bus == ON` ET `Pompe_Physique == 0`.
*   **Non-Dictature** : Le contrôleur suit le clavier via `target = real`.

---

### 3. Gestion Réseau & Sockets (La limite des 8-16)
L'ESP32 est limité en ressources réseau. Pour garantir la stabilité (Erreur 23) :
*   **Isolation Task (Anti-Blocking)** : Les requêtes HTTP ne doivent **JAMAIS** s'exécuter dans la tâche UART (Core 1). Elles doivent être déléguées à la boucle principale via le `App.scheduler`.
*   **Timeout Agressif** : Le timeout HTTP est fixé à **2s**. Une socket ne doit jamais rester ouverte inutilement.
*   **Fermeture Précoce** : L'option `capture_response: false` est impérative pour libérer les ressources immédiatement après l'envoi.
*   **Capacité** : `max_connections` fixé à **16** pour absorber les pics de trafic (API + Web + Eedomus).

---

### 4. Logique "Sniper"
*   Le Sniper effectue **3 à 5 tentatives** d'injection.
*   Il ne tire que si `Target != Real` ET que la file d'attente Eedomus est libre (priorité bus).

---

### 5. Dictionnaire des Bits
*   **0x1A** : Bit 0 (Filt), 1 (Heat), 2 (UVC), 3 (Ready).
*   **0x08** : Bit 0 (Pompe), 1 (Relais Chauffe).

---
*Sanctuarisé le 01/05/2026 - Version 6.9.30-PROD - Stable, Signé & Labo-Ready.*
