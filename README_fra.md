<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="Bannière d'ARMOR-NODE-SOLAR-HMI" width="100%">
</p>

# 🔋 ARMOR-NODE-SOLAR-HMI

<p align="center">
  <a href="README.md">🇺🇸 English</a> |
  <a href="README_spa.md">🇪🇸 Español</a> |
  🇫🇷 <b>Français</b> |
  <a href="README_ita.md">🇮🇹 Italiano</a> |
  <a href="README_deu.md">🇩🇪 Deutsch</a> |
  <a href="README_zho.md">🇨🇳 简体中文</a> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### L'écran d'énergie de la maison : un écran tactile de 7 pouces qui montrera les batteries, leurs cellules et les onduleurs tels qu'ARMOR-SERVER les connaît

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 ou ultérieure">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Langage">
  <img src="https://img.shields.io/badge/Board-ESP32--P4%207%20inch%20(planned)-e7352c.svg" alt="Board">
  <img src="https://img.shields.io/badge/Tests-5%20host%20programs-2ea44f.svg" alt="Tests">
  <img src="https://img.shields.io/badge/Version-0.0.1-0f6cbd.svg" alt="Version">
  <img src="https://img.shields.io/badge/Maturity-scaffolding-FFB020.svg" alt="Maturité">
</p>

---

**Vérification d'honnêteté - ce qui fonctionne aujourd'hui :** La logique de l'écran et un simulateur d'installation sont écrits et testés sur un ordinateur (5 programmes hôte). **Il n'y a pas encore de firmware, d'écrans ni de connexion à un serveur ; rien ici n'envoie d'ordre à une batterie et rien n'a tourné sur une carte.**

---

## 📋 En un coup d'œil

| | |
|---|---|
| Version | `0.0.1` |
| Maturité | Ébauche |
| Rôle | Écran tactile solaire d'A.R.M.O.R. (famille : Clients, parent : ARMOR-SERVER) |
| Technologie | Cœur et simulateur en C++17, CMake ; contrôles en Python |
| Tests | 5 programmes hôte |
| Carte | Guition JC1060P470C (ESP32-P4 avec ESP32-C6, 7 pouces, 1024 x 600) ; pas encore de code carte |
| Fonctionne sur | Un ordinateur avec CMake, un compilateur C++17 et Python 3.11+ |
| Sur matériel réel | Rien n'a tourné sur la carte |
| Licence | GPL-3.0-or-later |

**Aller à :** [Présentation](#-présentation) · [Architecture](#-architecture) · [Fonctionnalités](#-fonctionnalités) · [Sécurité](#-sécurité-et-protection) · [Démarrage rapide](#-démarrage-rapide) · [Tests](#-tests) · [Structure](#-structure-du-dépôt) · [Liés](#-projets-liés)

## 🚦 État du projet

Chaque ligne est une partie de ce dépôt et le feu dit où elle en est aujourd'hui. Elle est écrite d'après ce que le dépôt peut montrer (tests, compilations, exécutions), pas d'après ce qui est prévu.

🟢 Fait et vérifié · 🟡 Fonctionne, avec des limites connues · 🟠 À moitié fait ou pas encore vérifié · 🔴 Pas fait ou ne fonctionne pas

| Domaine | État | Ce que cela veut dire ici |
|---|---|---|
| Logiciel | 🟠 | Le cœur qui lit, juge et protège est écrit ; pas le firmware. |
| Tests | 🟡 | Programmes hôte et deux contrôles avec ARMOR-COMMON ; stade précoce. |
| Interface | 🔴 | Aucun écran n'est encore dessiné. |
| Matériel | 🔴 | La carte n'a pas de code et rien n'y a tourné. |
| Sécurité | 🟠 | PIN et contrôle d'écriture ne sont que de la logique testée ; aucune revue indépendante. |
| Mises à jour | 🔴 | Il n'y a encore rien à mettre à jour. |
| Intégration | 🔴 | Pas encore de lien avec ARMOR-SERVER ; la route solaire compacte nécessaire n'existe pas. |
| Documentation | 🟡 | README en huit langues et la note de conception. |

## 🎯 Présentation

* Montre le côté énergie d'une installation : totaux, batteries, cellules et onduleurs.
* Lit tout depuis le serveur par le réseau ; aucun câble série ni batterie.
* Passe en lecture seule, et le dit, quand les données sont anciennes.

## 🧩 Rôle dans A.R.M.O.R.

[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR) lit les équipements, [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER) garde ce qu'ils disent et décide de chaque ordre, et cet écran l'affiche. [ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI) est le panneau de tout le système ; celui-ci ne concerne que l'énergie.

## 🏛️ Architecture

<p align="center">
  <img src="images/architecture.svg" alt="Architecture d'ARMOR-NODE-SOLAR-HMI : le simulateur, le cœur testé et le serveur" width="100%">
</p>

## ✨ Fonctionnalités

* Une valeur absente ou impossible n'est pas affichée, jamais comme un zéro.
* Cellules, modules et batteries ont un niveau avec icône et mot ; seule une alarme clignote, deux fois par seconde au plus.
* PIN conservé sous forme de hachage salé, avec blocages croissants et session courte.
* Une opération batterie n'est proposée que si le serveur l'autorise, les données sont récentes, le PIN est ouvert et la personne a confirmé.
* Simulateur à cinq scénarios, vérifié contre les schémas des messages.

## 🔒 Sécurité et protection

| Menace | Défense |
|---|---|
| Quelqu'un devine le PIN | Blocages croissants qui survivent à un redémarrage ; session courte qui s'arrête sans contact |
| Agir sur des données anciennes | Sans réponse récente, l'écran ne fait que lire et ne propose rien |
| La flash est lue | Un PIN court n'y résiste pas : la carte devra chiffrer sa flash ; le serveur décide toujours de chaque ordre |

## 🚀 Démarrage rapide

**Prérequis :** CMake 3.16+, un compilateur C++17 et Python 3.11+.

```powershell
python tools/armor_project_tool.py build-test .  # les tests hôte (CMake et un compilateur C++17)
build/host/solar_sim cell-high                   # une réponse du simulateur
```

## 🧪 Tests

```powershell
python tools/armor_project_tool.py build-test .
python -I tools/armor_ci_validate.py
```

| Fichier | Ce qu'il couvre |
|---|---|
| `test_solar_data.cpp` | ce qui est lu et ce qui est écarté |
| `test_solar_view.cpp` | données anciennes, niveaux, clignotement |
| `test_pin_gate.cpp` | hachage du PIN, blocages, session |
| `test_write_gate.cpp` | chaque raison qui bloque une opération |
| `test_plant.cpp` | les cinq scénarios |
| `check_simulator.py` | le simulateur respecte les schémas |
| `check_base_copies.py` | les fichiers partagés correspondent à leur source |

## 🏗️ Compilation et publication

Il n'y a pas encore de firmware ni de version publiée. Quand il y en aura, il sera compilé uniquement dans GitHub Actions.

## 📂 Structure du dépôt

<p align="center">
  <img src="images/repository-structure.svg" alt="Structure du dépôt de ARMOR-NODE-SOLAR-HMI" width="100%">
</p>

<details>
<summary>Version texte de l'arborescence</summary>

```text
ARMOR-NODE-SOLAR-HMI/
├── core/
│   ├── solar_data.hpp    ce qui est lu du serveur
│   ├── solar_view.hpp    données anciennes, niveaux, clignotement
│   ├── pin_gate.hpp      le PIN de l'écran
│   ├── write_gate.hpp    quand proposer une opération
│   ├── json.hpp          de la base de firmware commune
│   └── hmac_sha256.hpp   de la base de firmware commune
├── simulator/
│   ├── plant.hpp         cinq scénarios
│   └── main.cpp          affiche les réponses
├── tests/                tests hôte et contrôles de contrat
├── docs/                 conception et sources des figures
├── tools/                contrôles communs
├── images/               images des README
├── .github/              workflows et formulaires
├── armor.project.json    manifeste
├── CHANGELOG.md          changements
├── LICENSE               GPL-3.0-or-later
├── CONTRIBUTING.md       comment contribuer
├── CODE_OF_CONDUCT.md    code de conduite
├── SECURITY.md           signaler une vulnérabilité
└── SUPPORT.md            obtenir de l'aide
```

</details>

## 🗺️ Feuille de route et limites connues

**Limites connues aujourd'hui**

* **Ce qu'il fait**: Lit la réponse solaire du serveur, la juge et protège le PIN, sur un ordinateur.
* **Ce qu'il ne fait pas**: Dessiner un écran, se connecter, envoyer un ordre ou tourner sur la carte.

**Prochainement**

* Une réponse solaire compacte dans ARMOR-SERVER pour petits écrans, définie d'abord dans ARMOR-COMMON.
* Les écrans en 1024 x 600 dessinés par un simulateur sur ordinateur, en huit langues.
* La couche carte partagée avec ARMOR-NODE-HMI.
* La première mise sous tension de la carte.

## 🌍 Langues

Le README et les textes existent en anglais, espagnol, français, italien, allemand, chinois, japonais et portugais européen.

## 🔗 Projets liés

**A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) est un système de sécurité périmétrique composé de dépôts indépendants. Chacun a sa propre version, ses propres tests et son propre README ; voici la famille :

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Contrats de messages, validateurs, vecteurs de conformité et types générés
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Firmware du nœud de terrain pour ESP32-S3 avec trois radars et son propre panneau web
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Protocoles des onduleurs et batteries solaires et messages d'un nœud passerelle
* **[ARMOR-NODE-ELECTRICAL](https://github.com/JuanenRac/ARMOR-NODE-ELECTRICAL)** - Nœud électrique : compteurs, le message des mesures du réseau et les règles de commutation
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Nœud et centrale d'alarme : zones, armement, temporisations, sirène et PIN, avec ou sans le serveur
* **[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI)** - Panneau tactile : l'état du système sur un écran mural, armer et acquitter, et la maison de l'assistant vocal
* **ARMOR-NODE-SOLAR-HMI** (ce dépôt) - Écran tactile solaire : les batteries et les onduleurs tels qu'ARMOR-SERVER les connaît (début : pas encore de firmware)
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - Le réseau local : ses appareils, internet et ce qui change
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - Coordinateur central : télémétrie, alarmes, appareils, relevés solaires et caméras
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Console web : caméras, radar, alarmes, énergie solaire et concepteur de site 2D/3D
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - Client Android de l'opérateur avec radar 2D/3D en direct
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - Client opérateur iPhone
* **[ARMOR-WATCH](https://github.com/JuanenRac/ARMOR-WATCH)** - Client montre minimal : avis d'alarme et confirmation d'alarme
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - Politique d'inférence visuelle qui explique ses décisions et n'agit jamais
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Zones de caméra, masques et règles de détection pour la Jetson Orin NX
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - Intentions vocales hors ligne avec une confirmation impossible à falsifier
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - Boîtiers, électronique et matrice d'acceptation sur banc
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - Déploiement, banc d'essai CM5, sauvegarde et TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - Simulateur de télémétrie hors ligne avec des pannes reproductibles
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - Détecte, installe et met à jour les propres dépôts de l'écosystème
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - Architecture, base de sécurité et matrice des capacités

## 📚 Documentation

* [Conception](docs/DESIGN.md) : les règles et le plan
* [Matrice des capacités : ce qui est prouvé et ce qui ne l'est pas](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [Catalogue des projets : versions et dépendances entre les dépôts](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [Historique des modifications de ce dépôt](CHANGELOG.md)

## 🤝 Contribuer et communauté

* Avant une pull request, lancez `python tools/armor_project_tool.py build-test .` et `python -I tools/armor_ci_validate.py` : les deux doivent réussir. Voir [CONTRIBUTING.md](CONTRIBUTING.md) et le [code de conduite](CODE_OF_CONDUCT.md).
* Le README existe en huit langues et est généré à partir des sources d'ARMOR-DOCS : une modification du texte se fait là-bas.
* Une modification d'un message entre composants commence dans ARMOR-COMMON.
* Ne décrivez pas comme fonctionnant sur une carte ce qui a seulement été compilé.
* Signalez une vulnérabilité en privé : [SECURITY.md](SECURITY.md). Questions, idées et rapports : electrohobby3d@gmail.com ([SUPPORT.md](SUPPORT.md)).

## 👤 AUTEUR

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 LICENCE

GPL-3.0-or-later - voir [LICENSE](LICENSE).
