# Configuration Eedomus – Périphériques MSPA

**Pour un dépôt public (GitHub)** : aucune IP, aucun ID eedomus ni clé API dans ce fichier. Les valeurs réelles se configurent en local (Eedomus + `esphome/secrets.yaml`).

L’ESP envoie les états vers Eedomus en **push** (API Eedomus). Dans Eedomus : pas d’URL à saisir dans les « Valeurs », pas de XPATH ; méthode HTTP par défaut.

---

## Correspondance entités ESP ↔ Eedomus

Le code (`esphome/mspa-controller.yaml`) pousse une entité ESP vers **un** périphérique Eedomus identifié par son **ID numérique** (créé dans Eedomus). La table ci‑dessous décrit la **structure** ; les IDs réels sont à configurer côté Eedomus et dans le code / secrets, jamais dans le dépôt.

| Rôle (nom logique)     | Nom de l’entité dans l’ESP | Type de périphérique Eedomus |
|------------------------|----------------------------|-----------------------------|
| Filtration             | Filtration                 | Liste de valeurs (ON/OFF)    |
| Chauffage              | Chauffage                  | Liste de valeurs (ON/OFF)    |
| Mode bulles            | Mode bulles                | Liste de valeurs (Arret, Niveau1, 2, 3) |
| UVC                    | UVC                        | Liste de valeurs (ON/OFF)    |
| Verrouillage clavier   | Verrouillage clavier       | Liste de valeurs (ON/OFF)    |
| Température consigne   | Température consigne       | Nombre décimal (°C)          |
| Température eau        | Température eau             | Nombre décimal (°C)          |

**IDs eedomus** : le code lit les IDs depuis `esphome/secrets.yaml` uniquement (aucun ID dans le dépôt). Clés à renseigner : `eedomus_periph_filtration`, `eedomus_periph_chauffage`, `eedomus_periph_bulles`, `eedomus_periph_uvc`, `eedomus_periph_verrouillage`, `eedomus_periph_consigne_temp`, `eedomus_periph_temp_eau`. Voir `esphome/secrets.yaml.example`. Ne jamais committer `secrets.yaml`.

---

## Consigne température

- **Type Eedomus** : Nombre décimal, unité °C.
- **Comportement** : l’ESP pousse la valeur quand le clavier envoie une consigne (trame 0x04) ou quand l’utilisateur change la consigne dans l’UI ESP. Mises à jour limitées (throttle).

### Envoyer une consigne depuis Eedomus vers le spa

- Entité ESP : **number** `consigne_setpoint` (20–40 °C, pas 0,5).
- **URL** (exemple) : `http://<IP_ESP>/number/consigne_setpoint/set?value=38`
- **Méthode** : POST ou GET selon version ESPHome. Remplacer `<IP_ESP>` par l’IP actuelle de l’ESP (DHCP) et `38` par la consigne voulue.

---

## URLs API ESP (piloter l’ESP depuis Eedomus)

Les chemins utilisent les **id** définis dans `esphome/mspa-controller.yaml` (switches, select, number).

| Entité                 | id (dans le YAML) | Exemple d’URL (remplacer &lt;IP_ESP&gt;) |
|------------------------|-------------------|------------------------------------------|
| Filtration             | sw_f              | `http://<IP_ESP>/switch/sw_f/turn_on` ou `turn_off` |
| Chauffage              | sw_h              | `http://<IP_ESP>/switch/sw_h/turn_on` ou `turn_off` |
| UVC                    | sw_uvc            | `http://<IP_ESP>/switch/sw_uvc/turn_on` ou `turn_off` |
| Verrouillage clavier   | lock              | `http://<IP_ESP>/switch/lock/turn_on` ou `turn_off` |
| Mode bulles            | sel_b             | `http://<IP_ESP>/select/sel_b/set?option=Arret` (ou Niveau1, Niveau2, Niveau3) |
| Température consigne   | consigne_setpoint | `http://<IP_ESP>/number/consigne_setpoint/set?value=38` |

**IP** : l’ESP est en DHCP ; utiliser l’IP actuelle de l’ESP (routeur, ESPHome, etc.), jamais en dur dans le dépôt.
