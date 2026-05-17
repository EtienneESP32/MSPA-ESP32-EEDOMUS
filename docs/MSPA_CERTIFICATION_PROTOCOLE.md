# PROTOCOLE DE CERTIFICATION MSPA (QA)

*Ce protocole doit être exécuté dans son intégralité avant toute validation d'un nouveau firmware vers la branche de Production.*

## 📌 PRÉREQUIS MATÉRIELS & LOGICIELS
- Environnement LABO actif (ESP32 de Test + Simulateur branché).
- Le Simulateur doit exécuter le firmware `mspa-simulator.yaml` v1.8.2 ou supérieur (incluant le Mode Chaos).
- Script PowerShell `RUN_TESTS_EXTREMES.ps1` disponible dans le dossier `scratch\`.

---

## 🟢 PHASE 1 : FONCTIONS DE BASE & SYNCHRONISATION
| ID | Nom du Test | Procédure | Critère de Succès (Validation) |
|:---|:---|:---|:---|
| **A1** | **Limites Consigne** | Mettre consigne à 20°C puis 40°C via Web. | Le Simulateur doit recevoir et afficher `20` puis `40`. L'interface Web ne doit pas "glitcher". |
| **A2** | **Sync Ascendante** | Modifier la consigne manuellement sur le simulateur. | L'interface Web doit se mettre à jour en moins de 1.5s (Watchdog UI). |
| **A3** | **Relais Actionneurs**| Activer/Désactiver Filtre, Chauffe, Bulles, UVC depuis le Web. | Les LEDs/Statuts du simulateur doivent refléter exactement les actions. |

---

## 🟡 PHASE 2 : RÉSILIENCE & GESTION DES CONFLITS (Filtre Ghost)
| ID | Nom du Test | Procédure | Critère de Succès (Validation) |
|:---|:---|:---|:---|
| **B1** | **Bruit vs Consigne** | Activer `FORCE GHOST HEAT` sur le simulateur (Bruit asynchrone) + Changer la consigne depuis le Web. | La consigne Web doit passer. L'interface ne doit pas clignoter aléatoirement (Le Filtre d'Enveloppe fait son job). |
| **B2** | **Duel Physique** | Changer la consigne Web et Simu exactement en même temps. | La consigne physique (Simulateur) gagne. L'interface Web se réaligne. |
| **B3** | **Survie Alerte F1** | Activer `ALERTE FILTRE F1` sur le simulateur. | Le composant `filt_alert` passe à ON. Le Web passe en lecture seule / Erreur. |

---

## 🔴 PHASE 3 : TESTS EXTRÊMES (TSUNAMI & COUPURES)
*(Utiliser le script automatisé `scratch\RUN_TESTS_EXTREMES.ps1`)*

| ID | Nom du Test | Procédure | Critère de Succès (Validation) |
|:---|:---|:---|:---|
| **X1** | **Le Trou Noir** | Envoyer des commandes massives avec une IP Eedomus indisponible. | La fonction de `push` réseau ne doit pas ralentir le relais UART (Le `FreeRTOS` et le `http_request` asynchrone isolent le problème). |
| **X2** | **Coupure Câble** | Suspendre les envois du Simulateur (Silence radio 5s). | Les watchdogs `Lien Moteur` / `Lien Clavier` tombent en `OFF`. Ils repassent en `ON` dès réception de trames. |
| **X3** | **Tsunami (Chaos)** | Injection de **5000 octets bruts aléatoires** à 10Hz via l'API du simulateur. | Le buffer UART (1024o) purge l'excédent sans Kernel Panic. Le contrôleur (Core 0/1) continue de répondre au Web instantanément. |

---

## ✅ VALIDATION
Si un seul test échoue, la version est marquée comme **INSTABLE** et retournée en correction.
Si tous les tests réussissent, la version peut recevoir le tag **GOLD** (Production) ou **DR** (Disaster Recovery).
