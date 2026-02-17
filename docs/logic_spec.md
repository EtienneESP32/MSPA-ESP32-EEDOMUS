# Spécifications logiques du contrôleur MSPA (ESP32)

## 1. Hiérarchie des commandes (synchronisation)

- **Entité numérique** : eedomus + interface Web ESP32 = un seul organe de commande. Toute modification sur l’un est répercutée sur l’autre.
- **Priorité** : "Dernier qui parle a raison".
  - Ordre via eedomus/UI → ESP32 modifie le bus UART.
  - Bouton physique sur le clavier → ESP32 détecte, met à jour son état, informe eedomus.

## 2. Source de vérité (retour d’état)

- L’état affiché (eedomus/UI) = ce que le Spa **fait réellement**, pas ce qu’on souhaite.
- **Validation par le Spa** : un changement n’est confirmé que lorsque l’ESP32 a reçu la trame de confirmation du Spa (état 0x1A). Évite les clignotements si le Spa refuse (ex. sécurité manque d’eau).

## 3. Mode Lock (inhibition du clavier)

- **Comportement** : Lock ON → l’ESP32 fait pare-feu sur le bus UART.
- **Action** : trames du clavier interceptées ; si ordre (ex. bulles), l’ESP32 réécrit la trame avec l’état défini par le contrôleur avant envoi au Spa.
- **Exception** : au redémarrage (power cycle), le verrou est toujours OFF par défaut (accès manuel en panne domotique).

## 4. Gestion du conflit de boucle (anti-oscillation)

- **Commande impulsionnelle** : l’ESP32 n’injecte une modification que jusqu’à ce que le Spa ait confirmé. Ensuite il redevient transparent et laisse passer les trames du clavier sans modification.
- **Libération du bus** : une fois état réel (Spa) = état souhaité (eedomus), plus d’injection forcée.

## 5. Télémétrie température

- Envoi vers eedomus uniquement en cas de **variation significative** ou **heartbeat** fixe, pour limiter le trafic et la base de données.
