# Dépannage et analyse des logs

## Erreurs HTTP / socket (probe eedomus)

### Messages typiques

```
E (603195) esp-tls: Failed to create socket (family 2 socktype 1 protocol 0)
E (603201) transport_base: Failed to open a new connection: 32770
E (603204) HTTP_CLIENT: Connection failed, sock < 0
[E] http_request set Error flag: unspecified
[E] HTTP Request failed: ESP_ERR_HTTP_CONNECT
[E] http_request cleared Error flag
```

### Signification

- **Failed to create socket** : l’ESP n’arrive pas à ouvrir une nouvelle connexion TCP (family 2 = IPv4, socktype 1 = TCP). Le code d’erreur 32770 peut correspondre à une limite de ressources (sockets ou mémoire).
- **HTTP Request failed: ESP_ERR_HTTP_CONNECT** : la requête HTTP vers eedomus (probe ou push) a échoué avant même d’établir la connexion.
- **http_request cleared Error flag** : le composant HTTP remet son drapeau d’erreur à zéro pour la prochaine tentative.

### Causes possibles

1. **eedomus injoignable** : box éteinte, autre réseau, pare-feu, ou IP incorrecte dans `secrets.yaml`. Dans ce cas l’ESP passe en mode « unreachable » et le probe s’exécute toutes les **5 minutes** → ces messages reviennent toutes les 5 min.
2. **Manque de sockets côté ESP** : trop de connexions (UI ouverte, API, requêtes HTTP). L’ESP a un nombre limité de sockets ; si la limite est atteinte, « Failed to create socket » apparaît. En **prod sans UI** (`web_server` commenté), la pression est moindre.
3. **Moment du probe** : au boot, le premier probe a lieu après 90 s. Si à ce moment les sockets sont déjà saturés (UI + API), le premier probe peut échouer ; les suivants (120 s ou 5 min) réessaieront.

### Que faire

- Vérifier que la box eedomus est allumée et joignable (ping depuis le même réseau que l’ESP).
- En phase **dev** : fermer l’onglet UI quand tu ne t’en sers pas ; les réglages 90 s / 120 s et les deux `yield()` limitent la charge.
- En **prod** : commenter le bloc `web_server` pour libérer RAM/sockets ; ces erreurs devraient devenir rares si eedomus est sur le réseau.

---

## Erreurs httpd (serveur web)

### Message typique

```
E (76052) httpd: httpd_accept_conn: error in accept (23)
```

- **errno 23** (ENFILE) : trop de descripteurs (sockets) ouverts ; le serveur web ne peut plus accepter de nouvelles connexions.
- Même logique que ci-dessus : réduire la charge (fermer l’UI, moins de probes) ou désactiver le `web_server` en prod.

---

## Références

- Configuration eedomus : `docs/config_eedomus.md`
- Secrets et clés : `docs/secrets_reference.md`
- Changelog (réglages UI / probe) : `docs/CHANGELOG.md`
