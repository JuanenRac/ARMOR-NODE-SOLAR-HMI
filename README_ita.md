<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="Banner di ARMOR-NODE-SOLAR-HMI" width="100%">
</p>

# 🔋 ARMOR-NODE-SOLAR-HMI

<p align="center">
  <a href="README.md">🇺🇸 English</a> |
  <a href="README_spa.md">🇪🇸 Español</a> |
  <a href="README_fra.md">🇫🇷 Français</a> |
  🇮🇹 <b>Italiano</b> |
  <a href="README_deu.md">🇩🇪 Deutsch</a> |
  <a href="README_zho.md">🇨🇳 简体中文</a> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### Lo schermo dell'energia di casa: uno schermo touch da 7 pollici che mostrerà le batterie, le loro celle e gli inverter come li conosce ARMOR-SERVER

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 o successiva">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Linguaggio">
  <img src="https://img.shields.io/badge/Board-ESP32--P4%207%20inch%20(planned)-e7352c.svg" alt="Board">
  <img src="https://img.shields.io/badge/Tests-9%20host%20programs-2ea44f.svg" alt="Test">
  <img src="https://img.shields.io/badge/Version-0.0.2-0f6cbd.svg" alt="Versione">
  <img src="https://img.shields.io/badge/Maturity-scaffolding-FFB020.svg" alt="Maturità">
</p>

---

**Controllo di onestà - cosa funziona oggi:** La logica dello schermo, un simulatore di impianto e le schermate sono scritti e provati su un computer (9 programmi host), e un firmware per la scheda si compila, con accesso, lettura del server, impostazioni in flash, Wi-Fi e aggiornamenti firmati, tutto spento finché qualcuno non lo accende. **Non è mai girato su una scheda e nulla qui invia ordini a una batteria.**

---

## 📋 In breve

| | |
|---|---|
| Versione | `0.0.2` |
| Maturità | Impalcatura |
| Ruolo | Schermo touch solare di A.R.M.O.R. (famiglia: Clients, genitore: ARMOR-SERVER) |
| Tecnologia | Nucleo e simulatore in C++17, CMake; controlli in Python |
| Test | 9 programmi host |
| Scheda | Guition JC1060P470C (ESP32-P4 con ESP32-C6, 7 pollici, 1024 x 600); codice che si compila, mai eseguito su di essa |
| Gira su | Un computer con CMake, un compilatore C++17 e Python 3.11+ |
| Su hardware reale | Nulla è girato sulla scheda |
| Licenza | GPL-3.0-or-later |

**Vai a:** [Panoramica](#-panoramica) · [Architettura](#-architettura) · [Funzionalità](#-funzionalità) · [Sicurezza](#-sicurezza-e-protezione) · [Avvio rapido](#-avvio-rapido) · [Test](#-test) · [Struttura](#-struttura-del-repository) · [Correlati](#-progetti-correlati)

## 🚦 Stato del progetto

Ogni riga è una parte di questo repository e il semaforo dice a che punto è oggi. È scritta in base a ciò che il repository può mostrare (test, compilazioni, esecuzioni), non a ciò che è previsto.

🟢 Fatto e verificato · 🟡 Funziona, con limiti noti · 🟠 Fatto a metà o non ancora verificato · 🔴 Non fatto o non funzionante

| Area | Stato | Cosa significa qui |
|---|---|---|
| Software | 🟠 | Nucleo, schermate e un firmware con collegamento di lettura scritti; nulla è girato su una scheda. |
| Test | 🟡 | Programmi host e due controlli con ARMOR-COMMON; fase iniziale. |
| Interfaccia | 🟠 | Le schermate sono disegnate da un simulatore su computer; mai sul pannello. |
| Hardware | 🔴 | Il firmware si compila; nulla è girato sulla scheda. |
| Sicurezza | 🟠 | PIN, impostazioni sigillate e aggiornamenti firmati sono solo logica provata e codice compilato; nessuna revisione indipendente. |
| Aggiornamenti | 🔴 | Gli aggiornamenti firmati sono scritti e spenti; nessuna immagine firmata è pubblicata o installata. |
| Integrazione | 🔴 | Il firmware ha accesso e collegamento di lettura, spenti per difetto; provati al computer con il simulatore, mai da una scheda verso un server. |
| Documentazione | 🟡 | README in otto lingue e la nota di progetto. |

## 🎯 Panoramica

* Mostra la parte energetica di un impianto: totali, batterie, celle e inverter.
* Legge tutto dal server via rete; non ha cavi seriali né verso la batteria.
* Diventa di sola lettura, e lo dice, quando i dati sono vecchi.

## 🧩 Ruolo in A.R.M.O.R.

[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR) legge gli apparecchi, [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER) conserva ciò che dicono e decide ogni ordine, e questo schermo lo mostra. [ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI) è il pannello di tutto il sistema; questo riguarda solo l'energia.

## 🏛️ Architettura

<p align="center">
  <img src="images/architecture.svg" alt="Architettura di ARMOR-NODE-SOLAR-HMI: il simulatore, il nucleo provato e il server" width="100%">
</p>

## ✨ Funzionalità

* Un valore mancante o impossibile non viene mostrato, mai come zero.
* Celle, moduli e batterie hanno un livello con icona e parola; lampeggia solo un allarme, al massimo due volte al secondo.
* PIN conservato come hash con sale, con blocchi crescenti e sessione breve.
* Un'operazione sulla batteria è proposta solo se il server la consente, i dati sono recenti, il PIN è aperto e la persona ha confermato.
* Chi ha prodotto l'apparecchio e cosa dice la batteria su tre livelli: avviso, protezione e guasto dell'elettronica, in rosso fisso.
* Simulatore con sei scenari, verificato con gli schemi dei messaggi.
* Schermate 1024 x 600 in otto lingue, disegnate al computer dallo stesso codice del firmware.
* Schermata di controllo per il primo giorno: chip, memoria, pannello, tocco, I2C, rete via cavo, barre di colore e angoli.

## 🔒 Sicurezza e protezione

| Minaccia | Difesa |
|---|---|
| Qualcuno indovina il PIN | Blocchi crescenti che sopravvivono a un riavvio; sessione breve che termina senza tocchi |
| Si agisce su dati vecchi | Senza una risposta recente lo schermo legge soltanto e non propone operazioni |
| La flash viene letta | Un PIN corto non resiste: la scheda dovrà cifrare la flash; il server decide comunque ogni ordine |

## 🚀 Avvio rapido

**Requisiti:** CMake 3.16+, un compilatore C++17 e Python 3.11+.

```powershell
python tools/armor_project_tool.py build-test .  # i test host (servono CMake e un compilatore C++17)
build/host/solar_sim cell-high                   # una risposta del simulatore
```

## 🧪 Test

```powershell
python tools/armor_project_tool.py build-test .
python -I tools/armor_ci_validate.py
```

| File | Cosa copre |
|---|---|
| `test_solar_data.cpp` | cosa si legge e cosa si scarta |
| `test_solar_view.cpp` | dati vecchi, livelli, lampeggio |
| `test_pin_gate.cpp` | hash del PIN, blocchi, sessione |
| `test_write_gate.cpp` | ogni motivo che blocca un'operazione |
| `test_plant.cpp` | i sei scenari |
| `test_screen_text.cpp` | ogni testo nelle otto lingue |
| `test_server_link.cpp` | la lettura del server: interruzioni, rifiuti |
| `test_panel.cpp` | impostazioni, console seriale, sessione, verifica aggiornamento |
| `test_screen_model.cpp` | allarmi elencati, cifre, pin |
| `check_font_glyphs.py` | i font hanno ogni carattere |
| `check_simulator.py` | il simulatore rispetta gli schemi |
| `check_base_copies.py` | i file condivisi coincidono con l'origine |

## 🏗️ Compilazione e rilascio

La prima release contiene un file per variante (revisione dello schermo e del chip), compilato in GitHub Actions e scritto all'indirizzo 0 con `esptool`. **Non provata su hardware: non è mai girata su una scheda.** Il workflow `Screen pictures (simulator)` disegna le schermate.

## 📂 Struttura del repository

<p align="center">
  <img src="images/repository-structure.svg" alt="Struttura del repository di ARMOR-NODE-SOLAR-HMI" width="100%">
</p>

<details>
<summary>Versione testuale dell'albero</summary>

```text
ARMOR-NODE-SOLAR-HMI/
├── core/
│   ├── solar_data.hpp       ciò che si legge dal server
│   ├── solar_view.hpp       dati vecchi, livelli, lampeggio
│   ├── pin_gate.hpp         il PIN dello schermo
│   ├── write_gate.hpp       quando proporre un'operazione
│   ├── screen_text.hpp      testi in 8 lingue
│   ├── screen_model.hpp     allarmi e cifre
│   ├── server_link.hpp      legge il server
│   ├── panel_*.hpp          impostazioni, console, sessione
│   ├── update_policy.hpp    quale immagine installare
│   ├── board_p4_jc1060.h    pin della scheda
│   ├── json.hpp             dalla base firmware comune
│   └── hmac_sha256.hpp      dalla base firmware comune
├── main/
│   ├── ui.cpp               le pagine solari
│   ├── diag_ui.cpp          controllo scheda
│   ├── board.c              schermo, tocco, rete
│   ├── link.cpp             legge il server
│   ├── store.cpp            impostazioni in flash
│   ├── serial_console.cpp   la console seriale
│   ├── ota.cpp              aggiornamenti firmati
│   ├── app_main.cpp         avvio e registro
│   └── fonts/               font dello schermo
├── solar_wifi/
│   └── solar_wifi.c         Wi-Fi tramite l'ESP32-C6
├── simulator/
│   ├── plant.hpp            sei scenari
│   ├── main.cpp             stampa le risposte
│   └── screens/             immagini al PC
├── tests/                   test host e controlli di contratto
├── docs/                    progetto e sorgenti delle figure
├── CMakeLists.txt           progetto firmware
├── sdkconfig.defaults       impostazioni build
├── partitions.csv           mappa della flash
├── variants/                altro chip o pannello
├── tools/                   controlli comuni
├── images/                  immagini dei README
├── .github/                 workflow e moduli
├── armor.project.json       manifesto
├── CHANGELOG.md             modifiche
├── LICENSE                  GPL-3.0-or-later
├── CONTRIBUTING.md          come contribuire
├── CODE_OF_CONDUCT.md       codice di condotta
├── SECURITY.md              segnalare vulnerabilità
└── SUPPORT.md               ottenere aiuto
```

</details>

## 🗺️ Roadmap e limiti noti

**Limiti noti oggi**

* **Cosa fa**: Legge la risposta solare del server, la valuta, protegge il PIN, disegna le schermate al computer e compila un firmware che può accedere, conservare le impostazioni, usare il Wi-Fi e accettare un aggiornamento firmato, tutto spento per difetto e impostato dalla console seriale.
* **Cosa non fa**: Inviare ordini a una batteria, configurarsi dallo schermo o girare sulla scheda (mai provato).

**Prossimi passi**

* La prima accensione sulla scheda con la schermata di controllo.
* Provare sulla scheda l'accesso, il collegamento di lettura, il Wi-Fi e un aggiornamento.
* La configurazione dallo schermo invece che dalla console seriale.
* Lo strato di scheda condiviso con ARMOR-NODE-HMI.

## 🌍 Lingue

Il README e i testi esistono in inglese, spagnolo, francese, italiano, tedesco, cinese, giapponese e portoghese europeo.

## 🔗 Progetti correlati

**A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) è un sistema di sicurezza perimetrale fatto di repository indipendenti. Ognuno ha la propria versione, i propri test e il proprio README; ecco la famiglia:

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Contratti dei messaggi, validatori, vettori di conformità e tipi generati
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Firmware del nodo di campo per ESP32-S3 con tre radar e un proprio pannello web
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Protocolli di inverter e batterie solari e messaggi di un nodo gateway
* **[ARMOR-NODE-ELECTRICAL](https://github.com/JuanenRac/ARMOR-NODE-ELECTRICAL)** - Nodo elettrico: contatori, il messaggio delle letture della rete e le regole di manovra
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Nodo e centrale d'allarme: zone, inserimento, ritardi, sirena e PIN, con il server o senza
* **[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI)** - Pannello touch: lo stato del sistema su uno schermo a parete, attivare e riconoscere gli allarmi, e la casa dell'assistente vocale
* **ARMOR-NODE-SOLAR-HMI** (questo repository) - Schermo touch solare: batterie e inverter come li conosce ARMOR-SERVER (fase iniziale: un firmware compila, mai eseguito su una scheda)
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - La rete locale: i suoi dispositivi, internet e ciò che cambia
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - Coordinatore centrale: telemetria, allarmi, dispositivi, letture solari e telecamere
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Console web: telecamere, radar, allarmi, energia solare e progettista del sito 2D/3D
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - Client Android dell'operatore con radar 2D/3D in tempo reale
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - Client operatore per iPhone
* **[ARMOR-WATCH](https://github.com/JuanenRac/ARMOR-WATCH)** - Client orologio minimale: avvisi di allarme e conferma dell'allarme
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - Politica di inferenza visiva che spiega le sue decisioni e non agisce mai
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Zone delle telecamere, maschere e regole di rilevamento per la Jetson Orin NX
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - Intenti vocali offline con una conferma impossibile da falsificare
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - Contenitori, elettronica e matrice di accettazione da banco
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - Distribuzione, banco di prova CM5, backup e TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - Simulatore di telemetria offline con guasti ripetibili
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - Rileva, installa e aggiorna i repository stessi dell'ecosistema
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - Architettura, base di sicurezza e matrice delle capacità

## 📚 Documentazione

* [Progetto](docs/DESIGN.md): le regole e il piano
* [Matrice delle capacità: cosa è provato e cosa no](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [Catalogo dei progetti: versioni e dipendenze tra i repository](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [Cronologia delle modifiche di questo repository](CHANGELOG.md)

## 🤝 Contribuire e comunità

* Prima di una pull request esegui `python tools/armor_project_tool.py build-test .` e `python -I tools/armor_ci_validate.py`: devono passare entrambi. Vedi [CONTRIBUTING.md](CONTRIBUTING.md) e il [codice di condotta](CODE_OF_CONDUCT.md).
* Il README esiste in otto lingue ed è generato dalle sorgenti in ARMOR-DOCS: una modifica al testo si fa lì.
* Una modifica a un messaggio tra componenti parte da ARMOR-COMMON.
* Non descrivere come funzionante su una scheda ciò che è stato solo compilato.
* Segnala una vulnerabilità in privato: [SECURITY.md](SECURITY.md). Domande, idee e segnalazioni: electrohobby3d@gmail.com ([SUPPORT.md](SUPPORT.md)).

## 👤 AUTORE

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 LICENZA

GPL-3.0-or-later - vedi [LICENSE](LICENSE).
