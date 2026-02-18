# Référence complète des secrets MSPA

Ce document liste **toutes** les clés utilisées dans le code source (contrôleur et sniffer).  
Le fichier à renseigner en local est **`esphome/secrets.yaml`** (copier depuis `esphome/secrets.yaml.example`).  
**Ne jamais committer `secrets.yaml`** (il est dans `.gitignore`).

---

## Clés utilisées dans le code

| Clé | Fichier(s) | Usage |
|-----|-------------|--------|
| `wifi_ssid` | mspa-controller.yaml, mspa-uart-sniffer.yaml | SSID du réseau Wi‑Fi |
| `wifi_password` | mspa-controller.yaml, mspa-uart-sniffer.yaml | Mot de passe Wi‑Fi |
| `ap_ssid` | mspa-controller.yaml | SSID du point d’accès de secours (AP) |
| `ap_password` | secrets.yaml.example uniquement | Mot de passe de l’AP (optionnel, non utilisé dans le YAML actuel) |
| `eedomus_enabled` | mspa-controller.yaml | `"true"` ou `"false"` – activer/désactiver eedomus |
| `eedomus_host` | mspa-controller.yaml | IP ou hostname de la box eedomus |
| `eedomus_api_user` | mspa-controller.yaml | Utilisateur API eedomus |
| `eedomus_api_secret` | mspa-controller.yaml | Secret API eedomus |
| `eedomus_periph_filtration` | mspa-controller.yaml | ID du périphérique eedomus « Filtration » |
| `eedomus_periph_chauffage` | mspa-controller.yaml | ID du périphérique eedomus « Chauffage » |
| `eedomus_periph_bulles` | mspa-controller.yaml | ID du périphérique eedomus « Bulles » |
| `eedomus_periph_uvc` | mspa-controller.yaml | ID du périphérique eedomus « UVC » |
| `eedomus_periph_verrouillage` | mspa-controller.yaml | ID du périphérique eedomus « Verrouillage » |
| `eedomus_periph_consigne_temp` | mspa-controller.yaml | ID du périphérique eedomus « Consigne température » |
| `eedomus_periph_temp_eau` | mspa-controller.yaml | ID du périphérique eedomus « Température eau » |

**Optionnel (IP fixe)** : si tu ajoutes `manual_ip` dans le WiFi du contrôleur, tu peux définir dans `secrets.yaml` les clés `static_ip`, `gateway`, `subnet` (voir la fin de `secrets.yaml.example`).

---

## Fichier modèle : `esphome/secrets.yaml.example`

Le fichier **`esphome/secrets.yaml.example`** contient toutes ces clés avec des valeurs factices.  
Pour retrouver un `secrets.yaml` perdu :

1. Copier `esphome/secrets.yaml.example` vers **`esphome/secrets.yaml`**.
2. Remplacer chaque valeur par tes vraies données (Wi‑Fi, eedomus, IDs périphériques).
3. Ne jamais committer `secrets.yaml`.

---

## Où chaque secret est utilisé dans le code

- **mspa-controller.yaml**  
  - Lignes 6–15 : substitutions (tous les `!secret` eedomus et périphériques).  
  - Lignes 100–102 : `wifi_ssid`, `wifi_password`, `ap_ssid`.  
  - Les URLs eedomus (scripts) utilisent `eedomus_host`, `eedomus_api_user`, `eedomus_api_secret` et les `${periph_*}` (qui viennent des secrets).

- **mspa-uart-sniffer.yaml**  
  - Lignes 19–20 : `wifi_ssid`, `wifi_password` uniquement.  
  - L’AP du sniffer a un SSID en dur (`MSPA-SNIFFER-RECOVERY`), pas de secret.

- **docs/config_eedomus.md**  
  - Décrit la configuration eedomus et renvoie à `secrets.yaml` / `secrets.yaml.example` pour les IDs.

- **README.md**  
  - Explique la procédure : copier l’example en `secrets.yaml`, remplir, ne pas committer.
