# 📋 Procédure de Déploiement : MSPA GOLD PROD

**Cible :** Contrôleur de Production (RUT240)
**Version Certifiée :** v7.5.40-GOLD-PROD
**Date d'intervention :** 16/05/2026

---

## 🛠 Étape 1 : Rétablissement Électrique
1. **Réarmement PRCD** : Appuyer sur le bouton **RESET** du disjoncteur différentiel sur le câble d'alimentation du spa.
2. **Vérification Visuelle** : L'afficheur du spa doit s'allumer et l'ESP32 doit être alimenté (LED témoin).
3. **Attente Sync** : Attendre 2 minutes que le RUT240 et l'ESP32 stabilisent leur liaison WiFi.

---

## 🌐 Étape 2 : Validation Réseau
1. Connectez-vous à l'interface du **RUT240**.
2. Allez dans **Status > Network > Wireless**.
3. Vérifiez la présence du module (IP attendue : `192.168.1.226`).
4. **Signal Critique** : Vérifiez que le RSSI est supérieur à `-75 dBm`. Si le signal est trop faible, le flashage OTA risque d'échouer.

---

## 🚀 Étape 3 : Flashage du Firmware GOLD
1. Accédez à l'interface Web du spa : [http://10.0.6.2:81](http://10.0.6.2:81).
2. Faites défiler jusqu'à la section **OTA Update**.
3. Cliquez sur **Choose File** et sélectionnez :
   `mspa-controller-v7.5.40-GOLD-PROD.bin`
4. Cliquez sur **Update**.
5. **ATTENTION** : Ne fermez pas la page avant la fin du décompte. Le module va redémarrer automatiquement.

---

## ✅ Étape 4 : Vérification de Certification
Après le redémarrage, vérifiez les points suivants sur l'interface Web :
- [ ] **Titre** : Doit être `MSPA v7.5.40-GOLD-PROD`.
- [ ] **Lien Eedomus** : Doit être **Désactivé** (Grisé/OFF) par défaut.
- [ ] **Uptime** : Doit commencer à 0s.
- [ ] **Température** : Doit s'afficher correctement après ~10s.

---

## 🔗 Étape 5 : Mise en Service Eedomus
1. Basculez le switch **"Lien Eedomus"** sur **ON**.
2. Vérifiez sur votre portail Eedomus que les valeurs se mettent à jour.
3. Testez une commande (ex: Allumage Filtration) depuis Eedomus et vérifiez la réaction du spa.

---
**Note technique :** L'architecture Dual-Core Sanctuary est maintenant active. En cas de perte WiFi future, l'interface Web pourra être lente, mais le spa restera piloté en toute sécurité par le Core 1.
