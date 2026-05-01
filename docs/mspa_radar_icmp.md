# 📡 Fiche Technique : Radar ICMP (v6.3.13)

Le **Radar ICMP** est le cœur du système de blindage réseau introduit dans la version v6.3.13-STABLE. Il remplace le polling HTTP agressif par une surveillance asynchrone ultra-légère.

## 🧠 Le Problème (v6.3.11 et antérieures)
Auparavant, le contrôleur tentait d'ouvrir une connexion HTTP toutes les 5 minutes pour vérifier la présence de l'eedomus. Si la box était débranchée :
1.  La pile réseau (lwIP) tentait une résolution ARP/TCP.
2.  Cette opération pouvait durer jusqu'à **2 secondes**, gelant totalement le processeur de l'ESP32.
3.  Le **Watchdog** (timer de sécurité) détectait ce gel et redémarrait le module par sécurité.
4.  Résultat : un reboot toutes les 5 minutes et 2 secondes.

## 🛡️ La Solution : Radar ICMP Natif
En v6.3.13, nous utilisons un composant C++ dédié (`mspa_ping`) basé sur l'API `esp_ping` de l'ESP-IDF.

### Fonctionnement Asynchrone
Contrairement à une requête HTTP, le Radar ICMP fonctionne en arrière-plan :
- **Envoi** : L'ESP envoie un paquet ICMP "Echo Request" et **rend immédiatement la main** au reste du code (gestion du Spa).
- **Callback** : Lorsqu'une réponse arrive (ou après un timeout de 1s), une fonction de "rappel" met à jour l'état du capteur `eedomus_online`.
- **Zéro Gel** : Le processeur n'est jamais arrêté, même si la box ne répond pas.

### Rôle de "Gatekeeper" (Garde-Barrière)
Le Radar n'est pas seulement un capteur ; il agit comme un filtre de sécurité pour toutes les actions réseau :
```cpp
// Logique simplifiée dans le YAML
- if:
    condition:
      lambda: 'return id(eedomus_online).state;' // Le Radar doit être au VERT
    then:
      - script.execute: push_to_eedomus // Sinon, on n'essaie même pas
```

## 📈 Avantages
1.  **Stabilité Absolue** : Fin des reboots cycliques liés au réseau.
2.  **Légèreté** : Un Ping ICMP est traité par le matériel réseau, sans charger le CPU de l'eedomus.
3.  **Transparence** : L'interface web affiche clairement si la liaison domotique est opérationnelle.

---
*Document technique pour MSPA-Controller v6.3.13-STABLE.*
