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
sudo make install
```

Or with the helper script:

```bash
sudo ./install.sh
```

Package-friendly or staged install:

```bash
make install DESTDIR=/tmp/package-root PREFIX=/usr
```

### Installed files

- binary: `/usr/local/bin/mdp-generator` by default
- desktop entry: `/usr/local/share/applications/mdp-generator.desktop` by default

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
sudo make install
```

Ou avec le script d'installation :

```bash
sudo ./install.sh
```

Installation preparee pour empaquetage ou destination temporaire :

```bash
make install DESTDIR=/tmp/package-root PREFIX=/usr
```

### Fichiers installes

- binaire : `/usr/local/bin/mdp-generator` par defaut
- entree desktop : `/usr/local/share/applications/mdp-generator.desktop` par defaut

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
