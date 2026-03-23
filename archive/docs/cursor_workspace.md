# Cursor : Workspace, dossier, nouveau projet

## Workspace vs Dossier (Folder)

Dans Cursor (comme dans VS Code) :

| Terme | Signification |
|-------|----------------|
| **Dossier (Folder)** | Un répertoire sur ton disque (ex. `c:\cursor`). C’est la **racine** de ce que tu vois dans l’explorateur de fichiers (barre latérale gauche). |
| **Workspace** | Ce qui est **ouvert dans la fenêtre actuelle**. En pratique, c’est presque toujours **un seul dossier** = un projet. Tu peux aussi avoir un fichier `.code-workspace` qui regroupe plusieurs dossiers dans une même fenêtre (cas avancé). |

En résumé : **ouvrir un dossier = ouvrir un projet**. Ce dossier est ton workspace pour cette fenêtre.

---

## Sauvegarder ce projet (MSPA)

1. **Fichiers** : *File → Save All* (ou **Ctrl+K S**).
2. **Git** : si tu as des changements, *Source Control* (icône branche) → Commit → Push. Comme le dépôt est déjà poussé sur GitHub, ton projet est sauvegardé et versionné.

---

## Créer un nouveau projet (ex. BLE)

1. **Créer le dossier du projet** (en dehors de Cursor) :
   - Ouvrir l’Explorateur Windows.
   - Aller où tu veux (ex. `c:\` ou `C:\Users\ebesa\Documents`).
   - Clic droit → *Nouveau* → *Dossier*.
   - Donner un nom (ex. `ble-spa` ou `mon-projet-ble`).

2. **Ouvrir ce dossier dans Cursor** :
   - Dans Cursor : **File → Open Folder…** (ou **Ctrl+K Ctrl+O**).
   - Choisir le dossier que tu viens de créer → *Sélectionner un dossier*.
   - Cursor ferme le projet actuel (MSPA) et ouvre le nouveau dossier = **nouveau workspace**.

3. **Versioning** : dès que le projet a du code, initialiser Git (voir ci‑dessous) pour ne pas s’y perdre.

---

## Revenir au projet MSPA

- **File → Open Recent** → choisir le dossier du projet MSPA (ex. `c:\cursor`).

---

## Règle du dépôt : versioning dans tout nouveau code

Dans ce dépôt, une règle Cursor impose : **dès qu’on crée ou structure un nouveau projet (ou du nouveau code ailleurs), mettre en place le versioning** (Git + `.gitignore` adapté, premier commit). Ainsi on ne s’y perd pas.
