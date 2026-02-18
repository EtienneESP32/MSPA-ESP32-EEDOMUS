# Configuration Eedomus – Périphériques MSPA

**Pour un dépôt public (GitHub)** : aucune IP, aucun ID eedomus ni clé API dans ce fichier. Les valeurs réelles se configurent en local (Eedomus + `esphome/secrets.yaml`).

L’ESP envoie les états vers Eedomus en **push** (API Eedomus, HTTP). Pour piloter l’ESP depuis Eedomus : Eedomus utilise le **serveur web** de l’ESP (HTTP, port 80), pas l’API native ESPHome (port 3232). Dans Eedomus : pas d’URL à saisir dans les « Valeurs » pour le push ; pour les commandes vers l’ESP, les URLs pointent vers l’IP de l’ESP (voir ci‑dessous).

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

## Piloter l’ESP depuis Eedomus (HTTP, pas l’API native)

Eedomus appelle l’ESP via **HTTP** (serveur web, port 80), pas via l’API native ESPHome (port 3232). Les URLs ci‑dessous sont des requêtes HTTP GET/POST vers le serveur web de l’ESP.

| Entité                 | id (dans le YAML) | Exemple d’URL (remplacer &lt;IP_ESP&gt;) |
|------------------------|-------------------|------------------------------------------|
| Filtration             | sw_f              | `http://<IP_ESP>/switch/filtration/turn_on` ou `turn_off` |
| Chauffage              | sw_h              | `http://<IP_ESP>/switch/chauffage/turn_on` ou `turn_off` |
| UVC                    | sw_uvc            | `http://<IP_ESP>/switch/uvc/turn_on` ou `turn_off` |
| Verrouillage clavier   | lock              | `http://<IP_ESP>/switch/verrouillage_clavier/turn_on` ou `turn_off` |
| Mode bulles            | sel_b             | `http://<IP_ESP>/select/mode_bulles/set?option=Arret` (ou Niveau1, Niveau2, Niveau3) |
| Température consigne   | consigne_setpoint | `http://<IP_ESP>/number/consigne_setpoint/set?value=38` |

**IP** : utiliser l’IP de l’ESP (fixe via `secrets.yaml` + `manual_ip`, ou DHCP). Ne pas mettre d’IP en dur dans le dépôt.

---

## Création des périphériques dans Eedomus

1. **Créer un périphérique** par entité (Filtration, Chauffage, Bulles, UVC, Verrouillage, Consigne temp., Temp. eau).
2. **Type** : Liste de valeurs (ON/OFF) pour les switches ; Nombre décimal pour consigne et température eau.
3. **Valeurs (liste)** : pour Filtration par ex. définir 0 = OFF, 1 = ON. Optionnel : renseigner l’URL (ex. `http://<IP_ESP>/switch/filtration/turn_on`) si tu veux que Eedomus appelle l’ESP ; sinon l’ESP pousse seul la valeur (push).
4. **Pas de XPATH** : laisser Requête / XPATH vides. L’ESP envoie les données en push vers Eedomus (API `periph.value`).
5. **Récupérer les IDs** : après création, noter l’ID numérique de chaque périphérique et le mettre dans `esphome/secrets.yaml` (clés `eedomus_periph_*`). Voir `docs/secrets_reference.md`.
