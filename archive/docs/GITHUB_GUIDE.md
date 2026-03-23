# Guide simple : publier ce projet sur GitHub

Tu peux faire tout depuis le navigateur et le terminal. Aucun menu compliqué dans Cursor : on utilise GitHub.com et la ligne de commande.

---

## Étape 1 : Créer un compte GitHub (si besoin)

1. Va sur **https://github.com**
2. **Sign up** → tu crées un compte (email + mot de passe).

---

## Étape 2 : Créer un dépôt vide sur GitHub

1. Connecte-toi sur **https://github.com**
2. En haut à droite : clic sur **« + »** → **« New repository »**
3. Remplis :
   - **Repository name** : par ex. `mspa-controller` (ou le nom que tu veux)
   - **Description** (optionnel) : ex. "Pont UART ESP32 pour spa MSPA + eedomus"
   - **Public**
   - Ne coche **pas** "Add a README" (on en a déjà un)
4. Clique sur **« Create repository »**
5. La page suivante affiche des commandes. **Ne les exécute pas tout de suite** si tu as déjà un dossier local.

---

## Étape 3 : Préparer ton dossier local (dans Cursor / terminal)

Ouvre un terminal **dans le dossier du projet** (par ex. `c:\cursor`).

```powershell
cd c:\cursor
git init
git add .
git commit -m "Initial: contrôleur MSPA + doc + sniffer"
```

(Si `git` n’est pas installé : installe **Git for Windows** depuis https://git-scm.com puis rouvre le terminal.)

---

## Étape 4 : Relier le dépôt GitHub et pousser

1. Sur la page de ton dépôt GitHub (celle créée à l’étape 2), repère l’URL du dépôt :
   - **HTTPS** : `https://github.com/TON_PSEUDO/mspa-controller.git`
   - ou **SSH** : `git@github.com:TON_PSEUDO/mspa-controller.git`
2. Dans ton terminal (toujours dans `c:\cursor`) :

```powershell
git remote add origin https://github.com/TON_PSEUDO/mspa-controller.git
git branch -M main
git push -u origin main
```

3. Si GitHub te demande de t’identifier : utilise ton **pseudo** et un **Personal Access Token** (mot de passe) au lieu du mot de passe du compte.
   - Pour créer un token : GitHub → **Settings** (ton profil) → **Developer settings** → **Personal access tokens** → **Generate new token** → coche `repo` → génère et copie le token, colle-le quand le terminal demande le mot de passe.

Après le `git push`, ton code est sur GitHub. Tu peux rafraîchir la page du dépôt : tu verras tous les fichiers.

---

## Où regarder quoi sur GitHub

| Où cliquer | Ce que tu vois |
|------------|-----------------|
| **Code** (onglet par défaut) | Liste des fichiers et dossiers du projet |
| **README.md** | S’affiche en bas de la page d’accueil du dépôt |
| **docs/** | Ouvre le dossier puis les `.md` (protocole, spec, tests) |
| **esphome/** | Les YAML (contrôleur + sniffer) |
| **Commits** | Historique des envois (chaque "commit") |
| **Settings** (si c’est ton dépôt) | Nom du dépôt, visibilité, suppression, etc. |

---

## Mettre à jour le dépôt plus tard

Après avoir modifié des fichiers dans Cursor :

```powershell
cd c:\cursor
git add .
git commit -m "Description courte de ce que tu as changé"
git push
```

Tu n’as pas besoin de menus spéciaux dans Cursor : tout se fait avec ces commandes et le site GitHub.
