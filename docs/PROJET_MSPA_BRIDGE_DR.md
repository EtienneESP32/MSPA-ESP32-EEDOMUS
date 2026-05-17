# 📂 Projet : MSPA-BRIDGE-DR (Disaster Recovery)
**Statut** : Spécification de Conception (V1.0)
**Philosophie** : "Shunt Logique" & Observation Passive

---

## 1. Vision et Objectif
Le projet **MSPA-BRIDGE-DR** vise à créer un micrologiciel de secours ultime pour les contrôleurs de spa MSPA basés sur ESP32. Contrairement à la version "GOLD" qui agit comme un médiateur actif (interception et injection), la version **DR** doit se comporter comme un **simple câble transparent** (Bridge), tout en conservant une capacité de diagnostic avancée.

L'objectif est d'éliminer toute suspicion sur l'ESP32 en cas de dysfonctionnement du clavier physique, en garantissant une latence nulle et une intégrité totale du flux UART original.

---

## 2. Piliers Fondamentaux (Design Principles)

### A. Le Shunt Logique (Relais Inconditionnel)
Le relais des données entre le Clavier (KBD) et le Bloc Moteur (SPA) doit être **non-bloquant** et **non-filtré**. 
*   Chaque octet reçu sur un port UART doit être immédiatement réémis sur l'autre port sans attendre la fin d'une trame ou la validation d'un checksum.
*   Aucune décision logique (Drop de trame, filtrage de commande) ne doit interférer avec le flux brut.

### B. L'Observation Passive (Mute Mode)
L'ESP32 devient une "sonde" pure.
*   **Zéro Injection** : Toutes les fonctions d'émission de commandes (Sniper, Reset Alerte, etc.) sont physiquement ou logiquement désactivées. 
*   L'ESP n'a plus l'autorisation d'écrire de lui-même sur le bus. Il ne fait que relayer ce qu'il entend.

### C. Le Mariage Sniffer-GOLD (Intelligence consultative)
Bien que le relais soit brut, l'analyse reste intelligente :
*   Le flux relayé est copié vers une machine d'état utilisant les filtres de la version **GOLD** (Filtre hybride d'enveloppe, Détection de Ghost Patterns).
*   Cette analyse permet de maintenir une interface Web et une remontée Eedomus fonctionnelles pour la consultation des états (Température, Mode, Alertes), sans jamais impacter la communication physique.

---

## 3. Architecture Technique et Isolation

### Isolation du Projet
*   **Composant Dédié** : Création d'un dossier `mspa_bridge` indépendant pour éviter tout conflit avec le code source certifié `mspa_uart`.
*   **Sanctuary Dual-Core** : Maintien de l'isolation des cœurs (Core 1 dédié au pont UART, Core 0 dédié aux services WiFi/Web/Eedomus) pour garantir qu'un ralentissement réseau ne puisse jamais impacter le relais UART.

### Comportement de la Boucle UART (Core 1)
1.  Lecture de l'octet entrant.
2.  Écriture immédiate vers la destination.
3.  Envoi de l'octet à la machine d'analyse passive (Post-Relais).

---

## 4. Objectifs de Validation (Tests)

Le projet devra être validé sur simulateur (GHOST Simulator) selon les axes suivants :
1.  **Test de Transparence** : Vérifier qu'une séquence de touches complexe sur le clavier arrive au spa avec une latence < 1ms.
2.  **Test de Non-Interférence** : Confirmer qu'aucune commande Eedomus n'est exécutée, même si demandée via l'interface Web.
3.  **Test de Résilience du Diagnostic** : Vérifier que sous un bruit de bus intense (H-Group), l'interface Web affiche toujours les états corrects grâce aux filtres GOLD, alors même que le relais transmet le bruit fidèlement.
