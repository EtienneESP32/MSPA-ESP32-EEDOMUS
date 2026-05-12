# Certification Technique : MSPA v7.5.38-DIAMOND-FINAL

**Date : 12 mai 2026**  
**Statut : REFERENCE PRODUCTION**

## 1. Philosophie "Sanctuary" (Architecture Dual-Core)
Le système repose sur une séparation stricte des responsabilités entre les deux cœurs de l'ESP32 :
- **Core 1 (Cœur Métier)** : Gère exclusivement l'UART (communication SPA/Clavier) et le Sniper. Fréquence 10Hz. Priorité absolue.
- **Core 0 (Cœur Service)** : Gère le WiFi, le serveur Web et les requêtes Eedomus. C'est ce cœur qui absorbe les latences réseau sans impacter la stabilité du bus.

## 5. Validation & Archivage PROD (12/05/2026)
- **Cible** : Production (Boîtier MSPA Réel).
- **Standard Pins** : 13 (TX) / 14 (RX) - *Vérifié et corrigé selon historique standard*.
- **Standard URLs** : Full Minuscules / Slugs (ex: `switch/filtration/turn_on`).
- **Sécurité Boot** : `eedomus_enabled: false` par défaut pour éviter tout bombardement au démarrage.
- **Binaire Archivé** : `stable_releases/2026-05-12_v7.5.37-DIAMOND-FINAL/mspa-controller-PROD-FINAL-1314.bin`.

**Signature : Antigravity AI - Certification DIAMOND délivrée le 12 mai 2026 à 01:45.**

## 2. Phase 1 : Règle de Non-Dictature (Anti-Rebond)
Pour résoudre le conflit entre l'automation et l'humain :
- **Logique** : Dès qu'une trame clavier (`!from_spa`) est détectée, le contrôleur aligne instantanément ses `targets` sur l'état `real` du bus.
- **Effet** : Si l'utilisateur éteint le chauffage sur le simulateur, le Sniper "dépose les armes" et ne tente plus d'imposer son ordre précédent.

## 3. Phase 2 : Double File d'Attente FIFO (Fluidité Eedomus)
Optimisation du flux sortant vers l'API Eedomus :
- **File d'Actions (Priorité Haute)** : Stocke les changements d'état (clics). FIFO strict.
- **File de Status (Priorité Basse)** : Stocke les températures et l'uptime. FIFO avec dédoublonnage agressif.
- **Dispatcher** : Envoie un message toutes les 5 secondes, en vidant systématiquement les Actions avant les Status.

## 4. Phase 3 : Filtre d'Enveloppe Hybride (Stabilité Visuelle)
La solution définitive contre le multiplexage visuel (clignotements) :
- **Détecteur d'Enveloppe** : Un bit vu à `1` reste verrouillé à `ON` pendant **3000ms**.
- **Priorité Physique** : SI un bit clignote (détecté par `is_blinking_`), ALORS l'état affiché suit le **moteur physique** (`0x08`) au lieu de la lampe.
- **Résultat** : En cas d'Alerte Filtre, le switch de filtration affiche **OFF** (vérité moteur) tandis que le capteur d'alerte affiche **ON** (vérité visuelle).

## 5. Protections Système (Fail-Safes)
- **Socket Guard** : Suspension des envois Eedomus si la Heap descend sous **48 Ko** (protection contre la saturation).
- **HTTP Timeout** : Libération forcée du verrou réseau après **10 secondes** en cas de gel de la pile TCP/IP.
- **Dédoublonnage cross-queue** : Avant d'ajouter une requête, le système nettoie les deux files pour éviter les ordres contradictoires.
## 6. Phase 4 : Arbitrage Diamond (Auto-Correction 10s)
Le mécanisme d'arbitrage assure une synchronisation parfaite entre l'UI et le bus UART :
- **Sniper Forcé** : Chaque clic UI déclenche 10 tirs, même si l'ESP pense être déjà à l'état cible (Régle du "Firm Grip").
- **Optimisme Stable** : L'interface réagit instantanément au clic pour une fluidité totale.
- **Watchdog d'Arbitrage** : Si l'état physique du bus ne correspond pas à la cible après 10 secondes, l'ESP effectue un **Revert** automatique.
- **Notification Eedomus** : Toute correction par arbitrage est immédiatement poussée vers l'Eedomus pour garantir l'intégrité de la domotique.

## 6. Historique des Flashs
- **v7.5.26** : Restauration Héritage.
- **v7.5.35** : Ajout Phases 1 et 3.
- **v7.5.36** : Correction Règle Hybride.
- **v7.5.37** : Implémentation Phase 2 (Double FIFO) et certification finale.

---
*Document généré automatiquement pour certifier l'intégrité de la version DIAMOND.*
