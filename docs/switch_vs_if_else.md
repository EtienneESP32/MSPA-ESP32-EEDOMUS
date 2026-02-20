# Switch vs if/else dans le code MSPA (C++)

## Où on peut utiliser `switch` dans le contrôleur

Le code tourne dans des lambdas ESPHome → C++. Une seule variable comparée à **plusieurs constantes entières** (IDs de trames, niveaux) se prête bien au `switch`.

| Bloc | Variable | Valeurs | Switch possible ? |
|------|----------|---------|-------------------|
| Lecture clavier (rid) | `rid` (uint8_t) | 0x01, 0x02, 0x19, 0x04, 0x03 | Oui |
| Bulles (rd) | `rd` (uint8_t) | 0, 1, 2, 3 | Oui |
| Réécriture (rid) | `rid` (uint8_t) | 0x01, 0x02, 0x19, 0x03, 0x04 | Oui |
| Option bulles (opt) | `opt` (string) | "Niveau1", "Niveau2", "Niveau3" | Non (pas de switch sur string en C++) |
| Lecture SPA (s_buf[1]) | `s_buf[1]` (uint8_t) | 0x06, 0x1A | Oui (2 cas seulement) |
| set_action bulles (x) | `x` (string) | "Niveau1", "Niveau2", "Niveau3" | Non |

---

## Avantages du `switch`

- **Lisibilité** : une seule expression testée, liste de cas claire (idéal pour IDs de trames).
- **Performance** : le compilateur peut générer une table de saut (jump table) → un seul branchement au lieu de plusieurs comparaisons (surtout utile pour beaucoup de cas).
- **Exhaustivité** : avec un `enum` ou des constantes, certains compilateurs peuvent avertir si un cas est oublié.
- **Maintenance** : ajouter un nouvel ID = ajouter un `case`, pas un `else if` au bon endroit.

---

## Inconvénients du `switch`

- **Types limités** : uniquement types entiers ou énumérés (`int`, `uint8_t`, `char`, `enum`). Pas de `switch` sur `string`, `float` ou conditions composées.
- **Pas de conditions** : pas de plages (`x >= 0 && x <= 10`) ni de tests complexes dans le `switch` ; il faut les mettre dans le `case` avec un `if` si besoin.
- **Fall-through** : sans `break`, l’exécution passe au `case` suivant → risque de bugs si on oublie le `break`.

---

## Quand garder `if`/`else`

- Comparaisons sur **chaînes** (`opt == "Niveau1"`, etc.) → rester en `if`/`else` (ou mapper string → int puis `switch` sur l’entier).
- **Conditions composées** : `if (rid == 0x01 && rd != last_c_h)` → soit `switch(rid)` avec `if (rd != last_c_h)` dans le `case`, soit garder les `if`/`else`.
- **Peu de cas** (2–3) : `switch` et `if`/`else` sont équivalents en clarté ; le `switch` peut quand même améliorer l’homogénéité du code (tous les IDs traités de la même façon).

---

## Résumé

- **Remplacer par `switch`** : les chaînes `if (rid == ...) ... else if (rid == ...)` et `if (rd == 0) ... else if (rd == 1)` dans la boucle UART.
- **Garder `if`/`else`** : tout ce qui porte sur des `string` ou des conditions non entières.
