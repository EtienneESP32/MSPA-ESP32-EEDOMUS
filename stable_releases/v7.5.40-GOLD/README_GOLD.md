# ARCHIVE GOLD v7.5.40 - MANIFESTE DE STABILITÉ

## 📜 Description
Cette archive contient la version finale, certifiée **PLATINUM**, du contrôleur MSPA. 
Elle a été sanctuarisée après une batterie de stress tests (Torture Suite) le 13 Mai 2026.

## 📁 Contenu du Sanctuaire
*   **src/** : Code source intégral (composant C++ et YAML).
*   **bin/** : Binaire prêt-à-flasher (firmware.bin).
*   **docs/** : Rapports de certification et protocoles de test.

## 💎 Clés de Stabilité (Rappel)
1.  **Dual-Core** : L'UART ne doit jamais quitter le Cœur 1.
2.  **Socket Guard** : Protection RAM active à 60ko.
3.  **Arbitrage Diamond** : Priorité absolue au bus physique sur le logiciel.
4.  **Filtre Enveloppe** : Latching de 3 secondes sur les switches pour ignorer le bruit de bus.

## 🚀 Comment restaurer ?
1.  Utiliser le binaire dans `bin/` via l'outil de flashage ESPHome.
2.  Les secrets (WiFi/Eedomus) sont déjà compilés à l'intérieur.

**Version certifiée par Antigravity AI.**
