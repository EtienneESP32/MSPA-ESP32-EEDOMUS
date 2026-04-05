# 🔎 Guide de Dépannage (Troubleshooting)

Si votre contrôleur ne répond pas comme prévu, ce guide vous aidera à identifier et résoudre les problèmes les plus courants rencontrés lors de l'installation.

---

## 1. Interprétation des Diagnostics (Codes 4b / 5b)

J'ai intégré des compteurs de diagnostic dans le firmware pour identifier les problèmes de bus UART en temps réel :

### 🔴 DIAG : Erreurs 4b (Erreurs de Trame)
*   **Signification** : L'ESP32 a reçu des données, mais il n'a pas pu identifier le début d'une trame valide (`0xA5`).
*   **Causes possibles** :
    *   **Inversion RX/TX** : Vérifiez que le TX du spa va bien vers le RX de l'ESP32 (via le level shifter).
    *   **Bruit sur la ligne** : Câbles de données trop longs ou mal blindés passant près de pompes à eau.
    *   **Vitesse incorrecte** : Le bus MSPA doit impérativement être à `9600 baud`.

### 🟡 DIAG : Erreurs 5b (Erreurs de Checksum)
*   **Signification** : La trame a été reçue, mais la somme de contrôle (4ème octet) ne correspond pas aux données.
*   **Causes possibles** :
    *   **Interférence ponctuelle** : Une courte perturbation sur le bus. Si ce chiffre reste faible (< 10 par heure), c'est négligeable.
    *   **Masse (GND) instable** : Vérifiez que le fil noir (GND) du Spa est bien connecté à toutes les broches GND de vos composants (ESP32 et Level Shifter).

---

## 2. Problèmes de Connexion & WiFi

### L'ESP32 ne se connecte pas au WiFi
*   **Cause** : Signal trop faible à cause du boîtier ou de la distance du spa.
*   **Solution** : L'ESP32 va créer son propre réseau WiFi de secours (nommé `mspa-controller` ou similaire). Connectez-vous dessus avec un smartphone pour accéder au **Captive Portal** et reconfigurer vos accès.

### L'ESP32 redémarre tout seul (Bootloop)
*   **Cause** : "Brownout" (chute de tension). Le 5V fourni par le spa peut chuter lors du démarrage du WiFi, faisant "tousser" l'ESP32.
*   **Solution** : Ajoutez un condensateur (ex: 100µF ou 470µF) entre les broches **5V** et **GND** de votre ESP32 pour lisser le courant.

---

## 3. Problème d'Interface (Eedomus / HA)

### Les états ne se mettent pas à jour
*   **Eedomus** : Vérifiez dans le log de l'ESP32 si vous voyez `HTTP Response: 200`. Si vous voyez un code `403` ou `401`, vos `api_user` ou `api_secret` dans `secrets.yaml` sont incorrects.
*   **Home Assistant** : Assurez-vous que l'intégration **ESPHome** est bien installée. L'auto-découverte peut prendre jusqu'à 2 minutes après le premier démarrage.

---

## 4. Comment vérifier si votre dépôt est Public ?

Pour être sûr que la communauté peut voir votre fabuleux travail sur GitHub :
1.  **Méthode Navigation Privée** : Ouvrez une fenêtre de navigation privée (sans être connecté à votre compte) et allez sur l'URL de votre dépôt. Si vous voyez le code, c'est **Gagné**.
2.  **Paramètres GitHub** : Allez dans `Settings` -> tout en bas `Danger Zone` -> `Change visibility`. Si le bouton indique "Make Private", c'est que votre dépôt est actuellement **PUBLIC**.

---

*Des questions supplémentaires ? Postez une "Issue" sur ce dépôt !*
