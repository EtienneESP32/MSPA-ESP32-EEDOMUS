# Configuration Eedomus – Périphériques MSPA

**Pour un dépôt public (GitHub)** : aucune IP, aucun ID eedomus ni clé API dans ce fichier. Les valeurs réelles se configurent en local (Eedomus + `esphome/secrets.yaml`).

L’ESP envoie les états vers Eedomus en **push** (API Eedomus, HTTP). Pour piloter l’ESP depuis Eedomus : Eedomus utilise le **serveur web** de l’ESP (HTTP, port 80), pas l’API native ESPHome (port 3232). Dans Eedomus : pas d’URL à saisir dans les « Valeurs » pour le push ; pour les commandes vers l’ESP, les URLs pointent vers l’IP de l’ESP (voir ci‑dessous).

---

## Correspondance entités ESP ↔ Eedomus

Dans le code (`esphome/mspa-controller.yaml`), chaque entité a :
- un **libellé lisible** (`name`) visible dans l’UI,
- un **id** (`id:`) utilisé en interne (lambdas/scripts),
- et un **slug URL** côté serveur web (souvent le `name` normalisé en minuscules + `_`).

Pour connaître le slug exact exposé par ton firmware, le plus fiable est d’ouvrir : `http://<IP_ESP>/json` et de regarder les champs `id` (ex. `switch/filtration`).

| Rôle (nom logique)     | Nom affiché (UI)            | id (YAML)          | Type Eedomus |
|------------------------|-----------------------------|--------------------|--------------|
| Filtration             | Filtration                  | sw_f               | Liste ON/OFF |
| Chauffage              | Chauffage                   | sw_h               | Liste ON/OFF |
| Mode bulles            | Mode bulles                 | sel_b              | Liste Arret, Niveau1/2/3 |
| UVC                    | UVC                         | sw_uvc             | Liste ON/OFF |
| Verrouillage clavier   | Verrouillage clavier        | lock               | Liste ON/OFF |
| Température consigne   | Temperature consigne        | consigne_setpoint  | Liste 25/30/35/36/37/38 °C |
| Température eau        | Temperature eau             | cur_temp           | Nombre °C |
| État réel filtration   | Etat reel filtration        | f_st               | (push seul, lecture) |
| État réel chauffage    | Etat reel chauffage         | h_st               | (push seul, lecture) |
| État réel bulles       | Etat reel bulles            | b_st               | (push seul, lecture) |

**IDs eedomus** : le code lit les IDs depuis `esphome/secrets.yaml` uniquement (clés `eedomus_periph_*`). Voir `esphome/secrets.yaml.example`. Ne jamais committer `secrets.yaml`.

---

## Consigne température

- **Côté ESP** : l’ESP pousse vers Eedomus la **valeur de consigne exacte** (celle du clavier ou de l’UI ESP), sans arrondi — par ex. 37, 37,5, 38. Push à chaque changement (clavier, trame 0x04, ou UI), avec throttle. **Cette valeur est celle qu’Eedomus reçoit et affiche** comme état actuel du périphérique (consigne affichée dans l’actionneur).
- **Côté Eedomus (actionneur)** : on configure un **actionneur de type Liste** avec **une ligne par degré** (25, 30, 35, 36, 37, 38) pour envoyer une consigne à l’ESP en un clic. L’affichage dans l’actionneur reflète la valeur poussée par l’ESP (exacte) ; les 6 options de la liste servent uniquement à *commander* une nouvelle consigne.

### Configurer l’actionneur « Consigne température » dans Eedomus

1. **Type** : Liste de valeurs (pas « Nombre »).
2. **Valeurs** : créer **6 lignes** (une par consigne utile) :

   | Nom affiché | ID valeur | URL (copier depuis `secrets.yaml`) |
   |-------------|-----------|-------------------------------------|
   | 25 °C       | 25        | `eedomus_url_consigne_25` |
   | 30 °C       | 30        | `eedomus_url_consigne_30` |
   | 35 °C       | 35        | `eedomus_url_consigne_35` |
   | 36 °C       | 36        | `eedomus_url_consigne_36` |
   | 37 °C       | 37        | `eedomus_url_consigne_37` |
   | 38 °C       | 38        | `eedomus_url_consigne_38` |

3. Pour chaque ligne : coller l’URL (méthode **POST**) depuis `esphome/secrets.yaml` (clés ci‑dessus). Pas besoin de tout mettre (ex. 20 à 40) : 25°, 30°, 35°, 36°, 37°, 38° couvrent l’usage typique (économie → confort).

---

## Piloter l’ESP depuis Eedomus (HTTP, pas l’API native)

Eedomus appelle l’ESP via **HTTP** (serveur web, port 80). Remplacer `<IP_ESP>` par l’IP de l’ESP.
Les slugs ci‑dessous correspondent au `name` normalisé (minuscules + `_`). Si doute, vérifier via `http://<IP_ESP>/json`.

| Entité (UI)             | Slug attendu | Exemple d’URL |
|-------------------------|--------------|---------------|
| Filtration              | filtration | `http://<IP_ESP>/switch/filtration/turn_on` ou `turn_off` |
| Chauffage               | chauffage | `http://<IP_ESP>/switch/chauffage/turn_on` ou `turn_off` |
| UVC                     | uvc | `http://<IP_ESP>/switch/uvc/turn_on` ou `turn_off` |
| Verrouillage clavier    | verrouillage_clavier | `http://<IP_ESP>/switch/verrouillage_clavier/turn_on` ou `turn_off` |
| Mode bulles             | mode_bulles | `http://<IP_ESP>/select/mode_bulles/set?option=Arret` (ou Niveau1, Niveau2, Niveau3). **Important** : valeurs exactes (Arret sans accent, Niveau1/2/3 sans espace). |
| Temperature consigne    | temperature_consigne | `http://<IP_ESP>/number/temperature_consigne/set?value=38` |

**IP** : utiliser l’IP de l’ESP (fixe via `secrets.yaml` + `manual_ip`, ou DHCP). Ne pas mettre d’IP en dur dans le dépôt.

---

## URLs pour actionneurs HTTP eedomus (copier-coller)

Les **URLs prêtes à coller** (avec l’IP de l’ESP) sont dans **`esphome/secrets.yaml`** (clés `eedomus_url_*`). Une valeur = une URL à coller dans l’état correspondant de l’actionneur HTTP. Méthode **POST**.

Correspondance rapide :
- **Filtration** : `eedomus_url_filtration_off` → état OFF, `eedomus_url_filtration_on` → état ON
- **Chauffage** : `eedomus_url_chauffage_off` / `eedomus_url_chauffage_on`
- **UVC** : `eedomus_url_uvc_off` / `eedomus_url_uvc_on`
- **Verrouillage clavier** : `eedomus_url_verrouillage_off` (débloqué) / `eedomus_url_verrouillage_on` (bloqué)
- **Bulles** : `eedomus_url_bulles_arret`, `eedomus_url_bulles_niveau1`, `niveau2`, `niveau3`
- **Consigne température** : une ligne par degré → `eedomus_url_consigne_25`, `_30`, `_35`, `_36`, `_37`, `_38` (voir section « Consigne température » ci‑dessus)

Voir `esphome/secrets.yaml.example` pour la liste des clés si tu recrées un `secrets.yaml`.

---

## Création des périphériques dans Eedomus

1. **Créer un périphérique** par entité (Filtration, Chauffage, Bulles, UVC, Verrouillage, Consigne temp., Temp. eau).
2. **Type** : Liste de valeurs (ON/OFF) pour les switches ; **Liste** pour la consigne température (6 lignes : 25, 30, 35, 36, 37, 38 °C) ; Nombre décimal pour la température eau (lecture seule push).
3. **Valeurs (liste)** : pour Filtration par ex. définir 0 = OFF, 1 = ON. Pour la consigne : 6 lignes (25 °C, 30 °C, … 38 °C), chaque ligne avec l’URL copiée depuis `secrets.yaml` (`eedomus_url_consigne_25` etc.). Pour les autres actionneurs : renseigner l’URL si tu veux que Eedomus appelle l’ESP ; sinon l’ESP pousse seul la valeur (push).
4. **Pas de XPATH** : laisser Requête / XPATH vides. L’ESP envoie les données en push vers Eedomus (API `periph.value`).
5. **Récupérer les IDs** : après création, noter l’ID numérique de chaque périphérique et le mettre dans `esphome/secrets.yaml` (clés `eedomus_periph_*`). Voir `docs/secrets_reference.md`.
