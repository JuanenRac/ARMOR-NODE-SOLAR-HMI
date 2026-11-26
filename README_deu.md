<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="ARMOR-NODE-SOLAR-HMI-Banner" width="100%">
</p>

# 🔋 ARMOR-NODE-SOLAR-HMI

<p align="center">
  <a href="README.md">🇺🇸 English</a> |
  <a href="README_spa.md">🇪🇸 Español</a> |
  <a href="README_fra.md">🇫🇷 Français</a> |
  <a href="README_ita.md">🇮🇹 Italiano</a> |
  🇩🇪 <b>Deutsch</b> |
  <a href="README_zho.md">🇨🇳 简体中文</a> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### Der Energiebildschirm des Hauses: ein 7-Zoll-Touchscreen, der die Batteriestapel, ihre Zellen und die Wechselrichter so zeigen wird, wie ARMOR-SERVER sie kennt

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 oder neuer">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Sprache">
  <img src="https://img.shields.io/badge/Board-ESP32--P4%207%20inch%20(planned)-e7352c.svg" alt="Board">
  <img src="https://img.shields.io/badge/Tests-5%20host%20programs-2ea44f.svg" alt="Tests">
  <img src="https://img.shields.io/badge/Version-0.0.1-0f6cbd.svg" alt="Version">
  <img src="https://img.shields.io/badge/Maturity-scaffolding-FFB020.svg" alt="Reifegrad">
</p>

---

**Ehrlichkeitsprüfung - was heute läuft:** Die Logik hinter dem Bildschirm und ein Simulator einer Anlage sind geschrieben und auf einem Computer getestet (5 Host-Programme). **Es gibt noch keine Firmware, keine Bildschirme und keine Verbindung zu einem Server; nichts hier sendet einen Befehl an eine Batterie, und nichts lief auf einer Platine.**

---

## 📋 Auf einen Blick

| | |
|---|---|
| Version | `0.0.1` |
| Reifegrad | Gerüst |
| Rolle | Solar-Touchscreen von A.R.M.O.R. (Familie: Clients, übergeordnet: ARMOR-SERVER) |
| Technik | Kern und Simulator in C++17, CMake; Prüfungen in Python |
| Tests | 5 Host-Programme |
| Platine | Guition JC1060P470C (ESP32-P4 mit ESP32-C6, 7 Zoll, 1024 x 600); noch ohne Platinencode |
| Läuft auf | Ein Computer mit CMake, C++17-Compiler und Python 3.11+ |
| Auf echter Hardware | Nichts lief auf der Platine |
| Lizenz | GPL-3.0-or-later |

**Springe zu:** [Überblick](#-überblick) · [Architektur](#-architektur) · [Funktionen](#-funktionen) · [Sicherheit](#-sicherheit-und-schutz) · [Schnellstart](#-schnellstart) · [Tests](#-tests) · [Struktur](#-struktur-des-repositorys) · [Verwandte](#-verwandte-projekte)

## 🚦 Projektstatus

Jede Zeile ist ein Teil dieses Repositorys, und die Ampel sagt, wie weit er heute ist. Sie beruht auf dem, was das Repository zeigen kann (Tests, Builds, Läufe), nicht auf dem, was geplant ist.

🟢 Fertig und überprüft · 🟡 Funktioniert, mit bekannten Grenzen · 🟠 Teilweise fertig oder noch nicht überprüft · 🔴 Nicht gemacht oder funktioniert nicht

| Bereich | Stand | Was das hier bedeutet |
|---|---|---|
| Software | 🟠 | Der Kern zum Lesen, Bewerten und Schützen ist geschrieben, die Firmware nicht. |
| Tests | 🟡 | Host-Programme und zwei Prüfungen gegen ARMOR-COMMON; frühes Stadium. |
| Oberfläche | 🔴 | Noch wird kein Bildschirm gezeichnet. |
| Hardware | 🔴 | Für die Platine gibt es keinen Code, nichts lief darauf. |
| Sicherheit | 🟠 | PIN und Schreibprüfung sind nur getestete Logik; keine unabhängige Prüfung. |
| Aktualisierungen | 🔴 | Es gibt noch nichts zu aktualisieren. |
| Integration | 🔴 | Noch keine Verbindung zu ARMOR-SERVER; die nötige kompakte Solar-Route fehlt. |
| Dokumentation | 🟡 | README in acht Sprachen und die Entwurfsnotiz. |

## 🎯 Überblick

* Zeigt die Energieseite einer Anlage: Summen, Batteriestapel, Zellen und Wechselrichter.
* Liest alles über das Netz vom Server; kein serielles Kabel und keines zur Batterie.
* Wird bei alten Daten nur lesend und sagt das auch.

## 🧩 Rolle in A.R.M.O.R.

[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR) liest die Geräte, [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER) hält fest, was sie melden, und entscheidet jeden Befehl; dieser Bildschirm zeigt es. [ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI) ist das Panel des ganzen Systems; dieses hier betrifft nur die Energie.

## 🏛️ Architektur

<p align="center">
  <img src="images/architecture.svg" alt="Architektur von ARMOR-NODE-SOLAR-HMI: Simulator, getesteter Kern und Server" width="100%">
</p>

## ✨ Funktionen

* Ein fehlender oder unmöglicher Wert wird nicht angezeigt, nie als Null.
* Zellen, Module und Stapel haben eine Stufe mit Symbol und Wort; nur ein Alarm blinkt, höchstens zweimal pro Sekunde.
* PIN als gesalzener Hash, mit wachsenden Sperren und kurzer Sitzung.
* Eine Batterieaktion wird nur angeboten, wenn der Server es erlaubt, die Daten frisch sind, die PIN offen ist und die Person bestätigt hat.
* Simulator mit fünf Szenarien, gegen die Nachrichtenschemas geprüft.

## 🔒 Sicherheit und Schutz

| Bedrohung | Abwehr |
|---|---|
| Jemand errät die PIN | Wachsende Sperren, die einen Neustart überdauern; kurze Sitzung, die ohne Berührung endet |
| Handeln nach alten Daten | Ohne frische Antwort liest der Bildschirm nur und bietet keine Aktion an |
| Der Flash wird ausgelesen | Eine kurze PIN hält dem nicht stand: Die Platine muss ihren Flash verschlüsseln; der Server entscheidet weiterhin jeden Befehl |

## 🚀 Schnellstart

**Voraussetzungen:** CMake 3.16+, ein C++17-Compiler und Python 3.11+.

```powershell
python tools/armor_project_tool.py build-test .  # die Host-Tests (braucht CMake und einen C++17-Compiler)
build/host/solar_sim cell-high                   # eine Antwort des Simulators
```

## 🧪 Tests

```powershell
python tools/armor_project_tool.py build-test .
python -I tools/armor_ci_validate.py
```

| Datei | Was sie abdeckt |
|---|---|
| `test_solar_data.cpp` | was gelesen und was weggelassen wird |
| `test_solar_view.cpp` | alte Daten, Stufen, Blinken |
| `test_pin_gate.cpp` | PIN-Hash, Sperren, Sitzung |
| `test_write_gate.cpp` | jeder Grund, der eine Aktion verhindert |
| `test_plant.cpp` | die fünf Szenarien |
| `check_simulator.py` | der Simulator folgt den Schemas |
| `check_base_copies.py` | gemeinsame Dateien entsprechen der Quelle |

## 🏗️ Build und Veröffentlichung

Es gibt noch keine Firmware und kein Release. Sobald es sie gibt, wird sie nur in GitHub Actions gebaut.

## 📂 Struktur des Repositorys

<p align="center">
  <img src="images/repository-structure.svg" alt="Repository-Struktur von ARMOR-NODE-SOLAR-HMI" width="100%">
</p>

<details>
<summary>Textversion des Baums</summary>

```text
ARMOR-NODE-SOLAR-HMI/
├── core/
│   ├── solar_data.hpp    was vom Server gelesen wird
│   ├── solar_view.hpp    alte Daten, Stufen, Blinken
│   ├── pin_gate.hpp      die PIN des Bildschirms
│   ├── write_gate.hpp    wann eine Batterieaktion erscheint
│   ├── json.hpp          aus der gemeinsamen Firmware-Basis
│   └── hmac_sha256.hpp   aus der gemeinsamen Firmware-Basis
├── simulator/
│   ├── plant.hpp         fünf Szenarien
│   └── main.cpp          gibt die Antworten aus
├── tests/                Host-Tests und Vertragsprüfungen
├── docs/                 Entwurf und Bildquellen
├── tools/                gemeinsame Prüfungen
├── images/               README-Bilder
├── .github/              Workflows, Formulare
├── armor.project.json    Manifest
├── CHANGELOG.md          Änderungen
├── LICENSE               GPL-3.0-or-later
├── CONTRIBUTING.md       wie man beiträgt
├── CODE_OF_CONDUCT.md    Verhaltenskodex
├── SECURITY.md           Schwachstellen melden
└── SUPPORT.md            Hilfe erhalten
```

</details>

## 🗺️ Roadmap und bekannte Grenzen

**Bekannte Grenzen heute**

* **Was es tut**: Liest die Solar-Antwort des Servers, bewertet sie und schützt die PIN, auf einem Computer.
* **Was es nicht tut**: Einen Bildschirm zeichnen, sich verbinden, Befehle senden oder auf der Platine laufen.

**Als Nächstes**

* Eine kompakte Solar-Antwort in ARMOR-SERVER für kleine Bildschirme, zuerst in ARMOR-COMMON festgelegt.
* Die Bildschirme in 1024 x 600, von einem Simulator am Computer gezeichnet, in acht Sprachen.
* Die mit ARMOR-NODE-HMI gemeinsame Platinenschicht.
* Das erste Einschalten auf der Platine.

## 🌍 Sprachen

README und Texte gibt es auf Englisch, Spanisch, Französisch, Italienisch, Deutsch, Chinesisch, Japanisch und europäischem Portugiesisch.

## 🔗 Verwandte Projekte

**A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) ist ein Perimeter-Sicherheitssystem aus unabhängigen Repositorys. Jedes hat eine eigene Version, eigene Tests und ein eigenes README; hier ist die Familie:

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Nachrichtenverträge, Validierer, Konformitätsvektoren und generierte Typen
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Feldknoten-Firmware für ESP32-S3 mit drei Radaren und eigenem Web-Panel
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Protokolle für Solar-Wechselrichter und -Batterien und die Nachrichten eines Gateway-Knotens
* **[ARMOR-NODE-ELECTRICAL](https://github.com/JuanenRac/ARMOR-NODE-ELECTRICAL)** - Elektroknoten: Zähler, die Nachricht der Netzmesswerte und die Regeln fürs Schalten
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Alarmknoten und Alarmzentrale: Zonen, Scharfschalten, Verzögerungen, Sirene und PIN, mit dem Server oder ohne ihn
* **[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI)** - Touch-Panel: der Systemzustand auf einem Wandbildschirm, Scharf- und Quittieren sowie das Zuhause des Sprachassistenten
* **ARMOR-NODE-SOLAR-HMI** (dieses Repository) - Solar-Touchscreen: Batterien und Wechselrichter, wie ARMOR-SERVER sie kennt (frühes Stadium: noch keine Firmware)
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - Das lokale Netzwerk: seine Geräte, das Internet und was sich ändert
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - Zentraler Koordinator: Telemetrie, Alarme, Geräte, Solarmesswerte und Kameras
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Web-Konsole: Kameras, Radar, Alarme, Solarenergie und 2D/3D-Standortdesigner
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - Android-Bedienclient mit Live-Radar in 2D/3D
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - iPhone-Bedienclient
* **[ARMOR-WATCH](https://github.com/JuanenRac/ARMOR-WATCH)** - Minimaler Uhren-Client: Alarmmeldungen und Alarmbestätigung
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - Visuelle Inferenzrichtlinie, die ihre Entscheidungen erklärt und nie handelt
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Kamerazonen, Masken und Erkennungsregeln für den Jetson Orin NX
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - Offline-Sprachabsichten mit einer nicht fälschbaren Bestätigung
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - Gehäuse, Elektronik und die Abnahmematrix am Prüfstand
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - Bereitstellung, CM5-Prüfstand, Backup und TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - Offline-Telemetriesimulator mit wiederholbaren Fehlern
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - Erkennt, installiert und aktualisiert die eigenen Repositorys des Ökosystems
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - Architektur, Sicherheitsgrundlage und die Fähigkeitsmatrix

## 📚 Dokumentation

* [Entwurf](docs/DESIGN.md): Regeln und Plan
* [Fähigkeitsmatrix: was belegt ist und was nicht](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [Projektkatalog: Versionen und wie die Repositorys voneinander abhängen](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [Änderungsverlauf dieses Repositorys](CHANGELOG.md)

## 🤝 Mitwirken und Community

* Führe vor einem Pull Request `python tools/armor_project_tool.py build-test .` und `python -I tools/armor_ci_validate.py` aus: Beides muss bestehen. Siehe [CONTRIBUTING.md](CONTRIBUTING.md) und den [Verhaltenskodex](CODE_OF_CONDUCT.md).
* Das README gibt es in acht Sprachen, und es wird aus den Quellen in ARMOR-DOCS erzeugt: Eine Textänderung erfolgt dort.
* Eine Änderung an einer Nachricht zwischen Komponenten beginnt in ARMOR-COMMON.
* Beschreibe nichts als auf einer Platine funktionierend, was nur gebaut wurde.
* Melde eine Schwachstelle privat: [SECURITY.md](SECURITY.md). Fragen, Ideen und Meldungen: electrohobby3d@gmail.com ([SUPPORT.md](SUPPORT.md)).

## 👤 AUTOR

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 LIZENZ

GPL-3.0-or-later - siehe [LICENSE](LICENSE).
