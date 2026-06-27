# Password Generator / Generateur de mots de passe

## English

`mdp-generator` is a desktop password generator for Ubuntu built with C++, GTK 3, OpenSSL, and ALSA.

The application is designed to provide a simple graphical interface while relying on microphone noise as its primary entropy source:

- the application captures 2 seconds of raw microphone noise through ALSA
- the captured audio buffer is hashed with SHA-256 through OpenSSL
- a pseudo-random byte stream is derived from that hash with a counter-based expansion
- the generator uses rejection sampling to avoid modulo bias
- when several character families are selected, the generated password includes at least one character from each selected family

### Features

- graphical desktop application for Ubuntu
- configurable password length
- selectable character families:
  - lowercase letters
  - uppercase letters
  - digits
  - symbols
- copy-to-clipboard button
- desktop launcher through a `.desktop` file
- clean install target for local installs or packaging

### Security model

This project does not rely on weak pseudo-random helpers such as `rand()` or `std::random_device` as its main source.

Instead, it captures physical microphone noise, hashes that recording with SHA-256, and expands the resulting digest into a pseudo-random stream that feeds the password generator.

### Dependencies

Install the required packages on Ubuntu:

```bash
sudo apt update
sudo apt install g++ pkg-config libgtk-3-dev libasound2-dev libssl-dev
```

### Build

```bash
make
```

This creates the executable:

```bash
./mdp-generator
```

### Run

```bash
./mdp-generator
```

### Tests

Run the non-UI tests:

```bash
make test
```

Generate a basic coverage report for the core password logic:

```bash
make coverage
```

### Install

System-wide install with the default prefix:

```bash
sudo make install PREFIX=/usr
```

Or with the helper script:

```bash
sudo ./install.sh
```

Package-friendly or staged install:

```bash
make install DESTDIR=/tmp/package-root PREFIX=/usr
```

### Debian package

Build a `.deb` package:

```bash
make deb
```

This creates a package in `dist/`, for example:

```bash
dist/mdp-generator_0.1.0_amd64.deb
```

Install it with:

```bash
sudo dpkg -i dist/mdp-generator_0.1.0_amd64.deb
```

GitHub publication:

- every push and pull request can build the `.deb` in GitHub Actions
- every tag like `v0.1.0` can publish the `.deb` as a GitHub Release asset
- every push and pull request can also build the `.snap` in GitHub Actions
- every tag like `v0.1.0` can also publish the `.snap` as a GitHub Release asset

### Ubuntu Software / App Center

If you want the application to appear in Ubuntu Software for everyone, the practical distribution format is a Snap package.

This repository now includes Snap packaging files in `snap/` and a `make snap` target.

Build the Snap locally:

```bash
make snap
```

Important notes:

- a `.deb` published on GitHub is useful for direct download, but it does not automatically appear in Ubuntu Software
- Ubuntu Software public distribution requires publication to the Snap Store
- because this application records microphone noise, the Snap declares the `audio-record` permission
- final public publication still requires a Snapcraft account and a manual store upload/review step

Typical publication flow:

```bash
snapcraft login
make snap
snapcraft upload --release=stable *.snap
```

### Reinstall after changes

If you modify the source code, the desktop launcher, or `mdp-logo.png`, you should reinstall the application before expecting the system menu or icon cache to reflect the update.

Typical cases where reinstall is required:

- the binary changed and you want the global `mdp-generator` command to use the new build
- the desktop file changed and you want the application launcher to be refreshed
- the icon changed and you want Ubuntu to pick up the new logo

Recommended command:

```bash
sudo ./install.sh
```

What this script does:

- installs the binary into `/usr/bin/mdp-generator`
- installs the desktop file into `/usr/share/applications/mdp-generator.desktop`
- installs the icon into `/usr/share/icons/hicolor/256x256/apps/mdp-logo.png`
- removes the old legacy copy from `/usr/local` if it exists
- refreshes the desktop and icon caches

If Ubuntu still shows the old icon after reinstall:

- close and reopen the application
- run `hash -r` in the shell before relaunching from terminal
- if the desktop shell still keeps the old icon cached, log out and log back in

### Installed files

- binary: `/usr/bin/mdp-generator` by default through `install.sh`
- desktop entry: `/usr/share/applications/mdp-generator.desktop` by default through `install.sh`
- icon: `/usr/share/icons/hicolor/256x256/apps/mdp-logo.png`

### User interface

The application window includes:

- a title describing the microphone-based generation flow
- a field to choose password length
- four checkboxes for character categories
- a generate button
- a read-only output field
- a copy button
- a status line explaining the capture or generation state

### Notes

- maximum password length is currently `1024`
- if you select several character types, the length must be at least the number of selected categories
- the launcher icon uses `mdp-logo.png`

## Francais

`mdp-generator` est un generateur graphique de mots de passe pour Ubuntu, developpe en C++, GTK 3, OpenSSL et ALSA.

L'application fournit une interface simple, mais repose sur le bruit du microphone comme source principale d'entropie :

- l'application capture 2 secondes de bruit micro brut via ALSA
- le tampon audio capture est hache en SHA-256 avec OpenSSL
- un flux pseudo-aleatoire est derive de ce hash par expansion avec compteur
- la selection des caracteres evite le biais de modulo
- quand plusieurs familles de caracteres sont cochees, le mot de passe genere contient au moins un caractere de chaque famille

### Fonctionnalites

- application graphique pour Ubuntu
- longueur du mot de passe configurable
- choix des familles de caracteres :
  - lettres minuscules
  - lettres majuscules
  - chiffres
  - symboles
- bouton de copie dans le presse-papiers
- lanceur desktop via un fichier `.desktop`
- cible d'installation propre pour usage local ou creation de paquet

### Modele de securite

Ce projet n'utilise pas des generateurs pseudo-aleatoires faibles comme `rand()` ou `std::random_device` comme source principale.

A la place, il capture du bruit physique via le microphone, calcule un hash SHA-256 de cet enregistrement, puis derive un flux pseudo-aleatoire qui alimente la generation du mot de passe.

### Dependances

Installe les paquets necessaires sous Ubuntu :

```bash
sudo apt update
sudo apt install g++ pkg-config libgtk-3-dev libasound2-dev libssl-dev
```

### Compilation

```bash
make
```

Cela produit l'executable :

```bash
./mdp-generator
```

### Lancement

```bash
./mdp-generator
```

### Tests

Lancer les tests non-UI :

```bash
make test
```

Generer un rapport de couverture simple pour la logique coeur :

```bash
make coverage
```

### Installation

Installation systeme avec le prefixe par defaut :

```bash
sudo make install PREFIX=/usr
```

Ou avec le script d'installation :

```bash
sudo ./install.sh
```

Installation preparee pour empaquetage ou destination temporaire :

```bash
make install DESTDIR=/tmp/package-root PREFIX=/usr
```

### Paquet Debian

Construire un paquet `.deb` :

```bash
make deb
```

Cela cree un paquet dans `dist/`, par exemple :

```bash
dist/mdp-generator_0.1.0_amd64.deb
```

Installation :

```bash
sudo dpkg -i dist/mdp-generator_0.1.0_amd64.deb
```

Publication sur GitHub :

- chaque push et pull request peut construire le `.deb` dans GitHub Actions
- chaque tag du type `v0.1.0` peut publier le `.deb` dans les assets d'une GitHub Release
- chaque push et pull request peut aussi construire le `.snap` dans GitHub Actions
- chaque tag du type `v0.1.0` peut aussi publier le `.snap` dans les assets d'une GitHub Release

### Ubuntu Software / App Center

Si tu veux que l'application apparaisse dans Ubuntu Software pour tout le monde, le format de distribution le plus adapte est un paquet Snap.

Le depot contient maintenant les fichiers d'empaquetage Snap dans `snap/` et une cible `make snap`.

Construire le Snap en local :

```bash
make snap
```

Points importants :

- un `.deb` publie sur GitHub est pratique pour le telechargement direct, mais il n'apparait pas automatiquement dans Ubuntu Software
- la distribution publique dans Ubuntu Software demande une publication dans le Snap Store
- comme l'application enregistre du bruit micro, le Snap declare la permission `audio-record`
- la publication publique finale demande quand meme un compte Snapcraft et une etape manuelle d'upload et de validation

Flux de publication typique :

```bash
snapcraft login
make snap
snapcraft upload --release=stable *.snap
```

### Reinstaller apres modification

Si tu modifies le code source, le lanceur desktop, ou `mdp-logo.png`, il faut reinstaller l'application avant d'attendre que le menu systeme ou le cache d'icones prenne en compte la mise a jour.

Cas typiques ou la reinstallation est necessaire :

- le binaire a change et tu veux que la commande globale `mdp-generator` utilise la nouvelle version
- le fichier desktop a change et tu veux rafraichir le lanceur
- l'icone a change et tu veux qu'Ubuntu prenne le nouveau logo

Commande recommandee :

```bash
sudo ./install.sh
```

Ce que fait ce script :

- installe le binaire dans `/usr/bin/mdp-generator`
- installe le fichier desktop dans `/usr/share/applications/mdp-generator.desktop`
- installe l'icone dans `/usr/share/icons/hicolor/256x256/apps/mdp-logo.png`
- supprime l'ancienne copie legacy dans `/usr/local` si elle existe
- rafraichit les caches desktop et icones

Si Ubuntu affiche encore l'ancienne icone apres reinstallation :

- ferme puis relance l'application
- execute `hash -r` dans le shell avant de relancer depuis le terminal
- si le shell graphique garde encore l'ancienne icone en cache, deconnecte-toi puis reconnecte-toi

### Fichiers installes

- binaire : `/usr/bin/mdp-generator` par defaut via `install.sh`
- entree desktop : `/usr/share/applications/mdp-generator.desktop` par defaut via `install.sh`
- icone : `/usr/share/icons/hicolor/256x256/apps/mdp-logo.png`

### Interface utilisateur

La fenetre contient :

- un titre qui decrit la generation basee sur le microphone
- un champ pour choisir la longueur
- quatre cases a cocher pour les categories de caracteres
- un bouton pour lancer la generation
- un champ de resultat en lecture seule
- un bouton pour copier
- une ligne d'etat qui explique la capture ou le resultat

### Remarques

- la longueur maximale est actuellement de `1024`
- si plusieurs types de caracteres sont selectionnes, la longueur doit etre au moins egale au nombre de categories choisies
- l'icone du lanceur utilise `mdp-logo.png`
