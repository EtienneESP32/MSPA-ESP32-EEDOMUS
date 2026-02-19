# Référence complète des secrets MSPA

Ce document liste **toutes** les clés utilisées dans le code source (contrôleur et sniffer).  
Le fichier à renseigner en local est **`esphome/secrets.yaml`** (copier depuis `esphome/secrets.yaml.example`).  
**Ne jamais committer `secrets.yaml`** (il est dans `.gitignore`).

---

## Avant le premier flash (checklist obligatoire)

**Ne pas flasher avec les valeurs d’exemple** : l’ESP ne se connectera pas au Wi‑Fi ni à eedomus. Dans `secrets.yaml`, remplacer **obligatoirement** :

| Clé | Remplacer par |
|-----|----------------|
| `wifi_ssid` | Nom exact de ton réseau Wi‑Fi |
| `wifi_password` | Mot de passe Wi‑Fi |
| `eedomus_host` | **IP réelle de la box eedomus** (ex. `192.168.1.242`), pas `IP_OU_HOST_EEDOMUS` |
| `eedomus_api_user` | Ton api_user eedomus |
| `eedomus_api_secret` | Ton api_secret eedomus |
| `eedomus_periph_*` | Les IDs numériques des périphériques créés dans eedomus |

Si `eedomus_host` reste en placeholder, les logs afficheront **Error parse url http://IP_OU_HOST_EEDOMUS/api/** et le push vers eedomus ne fonctionnera pas. Voir `docs/depannage_logs.md` section « Placeholder eedomus ».

---

## Clés utilisées dans le code

| Clé | Fichier(s) | Usage |
|-----|-------------|--------|
| `wifi_ssid` | mspa-controller.yaml, mspa-uart-sniffer.yaml | SSID du réseau Wi‑Fi |
| `wifi_password` | mspa-controller.yaml, mspa-uart-sniffer.yaml | Mot de passe Wi‑Fi |
| `ap_ssid` | mspa-controller.yaml | SSID du point d’accès de secours (AP) |
| `ap_password` | mspa-controller.yaml | Mot de passe du réseau de secours (AP MSPA-RECOVERY) |
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

**URLs eedomus (copier-coller)** : les clés `eedomus_url_*` ne sont pas lues par l’ESP ; elles contiennent les URLs complètes (avec l’IP de l’ESP) à coller dans les états des actionneurs HTTP eedomus. Consigne température = 6 clés (`eedomus_url_consigne_25` … `_38`). Voir `docs/config_eedomus.md` section « Consigne température » et « URLs pour actionneurs HTTP eedomus ».

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
