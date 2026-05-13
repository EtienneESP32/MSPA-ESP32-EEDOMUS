# Procédure de Stress-Test : MSPA v7.5.39-DIAMOND-REFRESH

Cette procédure vise à certifier l'invulnérabilité du module face aux conditions extrêmes (réseau VPN lent, saturation de commandes, bruit sur le bus).

## 1. Test de Charge UI (Saturation FIFO)
**Objectif** : Vérifier que le module ne crashe pas lors d'un "bombardement" de clics et que le Socket Guard protège la RAM.
*   **Action** : Exécuter la commande de bascule de la filtration 10 fois en moins de 5 secondes.
*   **Commande CLI** :
    ```powershell
    for ($i=0; $i -lt 10; $i++) { 
      py -m esphome switch.toggle filtration --device 192.168.1.226; 
      Start-Sleep -Milliseconds 200 
    }
    ```
*   **Résultat attendu** : 
    - Le module reste joignable sur le Web et via CLI.
    - Les logs affichent `Socket Guard` si la heap descend trop bas.
    - Aucune erreur de type `AsyncTCP` ou reboot.

## 2. Test d'Arbitrage (Échec Commande)
**Objectif** : Valider le mécanisme de "Revert" automatique sans boucle infinie.
*   **Pré-requis** : Débrancher temporairement le fil TX de l'ESP vers le SPA (ou bloquer logiciellement l'injection).
*   **Action** : Cliquer sur "Chauffage ON" dans l'UI.
*   **Observation** :
    - Le Sniper doit tenter ses 10 tirs.
    - Après 10 secondes exactes, le log doit afficher : `Diamond: Sync UI sur Bus (Auto-Correction Heat)`.
    - L'UI doit repasser d'elle-même sur OFF.
*   **Résultat attendu** : Synchronisation parfaite entre UI et réalité physique.

## 3. Test de Résilience Réseau (Simulation VPN Lent)
**Objectif** : Vérifier que le module survit à des délais de réponse HTTP extrêmes.
*   **Action** : Provoquer des échecs réseau ou saturer la file Eedomus.
*   **Observation** : 
    - Surveiller les messages `Component mspa_uart took a long time`.
    - Vérifier que malgré ces blocages sur le Core 0, les trames UART sur le Core 1 continuent de défiler fluidement (via les logs de debug).
*   **Résultat attendu** : Pas de crash, pas de saturation de la pile TCP.

## 4. Test de Non-Dictature (Priorité Humaine)
**Objectif** : Confirmer que le clavier physique a toujours le dernier mot.
*   **Action** : Lancer un cycle de Sniper (ex: Action Reset Alerte) et, pendant qu'il tourne, presser un bouton sur le clavier physique (ou simulateur).
*   **Observation** : Le Sniper doit s'arrêter immédiatement dès la détection de la trame clavier.
*   **Résultat attendu** : Arrêt instantané du Sniper.

## 5. Test du Détecteur d'Enveloppe (Anti-Ghost)
**Objectif** : Vérifier la stabilité visuelle lors de patterns de multiplexage agressifs.
*   **Action** : Utiliser le simulateur en mode "Realistic" (v1.8.2) pour générer des patterns de clignotement.
*   **Résultat attendu** : L'état des switchs dans ESPHome doit rester fixe (ON permanent) malgré le clignotement des bits sur le bus.
