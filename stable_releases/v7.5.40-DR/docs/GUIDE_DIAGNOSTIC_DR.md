# 🔍 Guide de Diagnostic : Mode Disaster Recovery (DR)

Ce document explique comment utiliser et interpréter la version **DR-BRIDGE** (Disaster Recovery) du contrôleur MSPA.

---

## 💡 Concept : Le Shunt Logique
En mode DR, l'ESP32 agit comme un **simple fil de cuivre** entre le clavier et le spa.
- **Relais Transparent** : Chaque octet est renvoyé à 100% sans vérification préalable.
- **Observation Passive** : L'ESP décode les trames pour ton information mais ne modifie rien.
- **Mute Mode** : L'ESP n'injecte aucune commande. Si tu cliques sur le Web, rien ne se passe sur le spa.

---

## 📋 Comment lire les Logs (Mode Diag)
En connectant les logs (port 6053 ou Série), voici les messages critiques que tu pourras observer :

### 1. Erreurs de Checksum (`Invalid Frame Checksum`)
- **Message** : `BRIDGE: Invalid KBD Frame Checksum! ID=XX`
- **Interprétation** : Le clavier a envoyé une donnée corrompue. Si cela arrive souvent, le câble du clavier ou le clavier lui-même est défaillant.
- **Impact** : En mode DR, cette donnée corrompue est **quand même relayée** au spa (comme un vrai fil), ce qui permet de voir comment le spa réagit physiquement au bruit.

### 2. Codes d'Erreur SPA (`SPA ERROR DETECTED`)
- **Message** : `!!! SPA ERROR DETECTED: E01 !!!`
- **Signification** :
    - **E01** : Capteur de température défaillant.
    - **E02** : Problème de flux d'eau (débitmètre).
    - **E03** : Température trop basse (risque gel).
- **Utilité** : Cela confirme que le bloc moteur du spa a détecté une panne interne, indépendamment de l'ESP32.

### 3. Latence et Perte de Lien
- **Message** : `Lien Moteur: False`
- **Interprétation** : L'ESP ne voit plus passer aucune trame en provenance du moteur depuis 3,5 secondes. Le bloc moteur est probablement éteint ou le câble est sectionné.

---

## 🛠 Procédure d'urgence
Si le clavier physique du spa ne répond plus en version GOLD :
1. Flash la version **DR-BRIDGE-PROD**.
2. Si le clavier ne répond toujours pas en version DR, alors la panne est **100% matérielle** (clavier HS ou câble oxydé).
3. Si le clavier répond en version DR mais pas en GOLD, alors la logique de filtrage GOLD est trop sévère pour ton environnement.

---
**Note** : En mode DR, l'Eedomus est un simple observateur. N'essaie pas d'automatiser des scénarios, ils seront ignorés par le spa.
