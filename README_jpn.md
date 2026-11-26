<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="ARMOR-NODE-SOLAR-HMI banner" width="100%">
</p>

# 🔋 ARMOR-NODE-SOLAR-HMI

<p align="center">
  <a href="README.md">🇺🇸 English</a> |
  <a href="README_spa.md">🇪🇸 Español</a> |
  <a href="README_fra.md">🇫🇷 Français</a> |
  <a href="README_ita.md">🇮🇹 Italiano</a> |
  <a href="README_deu.md">🇩🇪 Deutsch</a> |
  <a href="README_zho.md">🇨🇳 简体中文</a> |
  🇯🇵 <b>日本語</b> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### 家のエネルギー画面：バッテリースタック、セル、インバーターを ARMOR-SERVER が把握している通りに表示する予定の 7 インチタッチ画面

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 or later">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Language">
  <img src="https://img.shields.io/badge/Board-ESP32--P4%207%20inch%20(planned)-e7352c.svg" alt="Board">
  <img src="https://img.shields.io/badge/Tests-5%20host%20programs-2ea44f.svg" alt="Tests">
  <img src="https://img.shields.io/badge/Version-0.0.1-0f6cbd.svg" alt="Version">
  <img src="https://img.shields.io/badge/Maturity-scaffolding-FFB020.svg" alt="Maturity">
</p>

---

**正直さのチェック - 今日動いているもの:** 画面を支えるロジックと設備のシミュレータは作成済みで、コンピュータ上でテストしています（ホストプログラム 5 本）。**ファームウェア、画面、サーバーへの接続はまだありません。ここからバッテリーへ命令を送るものはなく、基板上で動いたものもありません。**

---

## 📋 ひと目で

| | |
|---|---|
| バージョン | `0.0.1` |
| 成熟度 | 足場段階 |
| 役割 | A.R.M.O.R. の太陽光タッチ画面（ファミリー：Clients、親：ARMOR-SERVER） |
| 技術 | C++17 のコアとシミュレータ、CMake、Python の検査 |
| テスト | ホストプログラム 5 本 |
| 基板 | Guition JC1060P470C（ESP32-P4 と ESP32-C6、7 インチ 1024 x 600）。基板用コードは未作成 |
| 動作環境 | CMake、C++17 コンパイラ、Python 3.11+ を備えた PC |
| 実機で | 基板上では何も動いていません |
| ライセンス | GPL-3.0-or-later |

**移動先：** [概要](#-概要) · [アーキテクチャ](#-アーキテクチャ) · [機能](#-機能) · [セキュリティ](#-セキュリティと安全) · [クイックスタート](#-クイックスタート) · [テスト](#-テスト) · [構成](#-リポジトリの構成) · [関連](#-関連プロジェクト)

## 🚦 プロジェクトの状況

各行はこのリポジトリの一部分で、信号は今日どこまで進んでいるかを示します。計画ではなく、リポジトリが示せるもの（テスト、ビルド、実行）に基づいて書かれています。

🟢 完了・検証済み · 🟡 動作する（既知の制限あり） · 🟠 一部のみ、または未検証 · 🔴 未実装または動作しない

| 領域 | 状態 | ここでの意味 |
|---|---|---|
| ソフトウェア | 🟠 | 読み取り・判定・保護のコアは作成済み。ファームウェアは未作成。 |
| テスト | 🟡 | ホストプログラムと ARMOR-COMMON に対する 2 つの検査。初期段階。 |
| インターフェース | 🔴 | 画面はまだ描画されません。 |
| ハードウェア | 🔴 | 基板用のコードはなく、何も動いていません。 |
| セキュリティ | 🟠 | PIN と書き込み判定はテスト済みロジックのみ。第三者レビューなし。 |
| アップデート | 🔴 | 更新する対象はまだありません。 |
| 連携 | 🔴 | ARMOR-SERVER との接続は未実装。必要な簡潔な太陽光ルートもありません。 |
| ドキュメント | 🟡 | 8 言語の README と設計メモ。 |

## 🎯 概要

* 設備のエネルギー面（合計、バッテリースタック、セル、インバーター）を表示します。
* すべてネットワーク経由でサーバーから読み取り、シリアルやバッテリーへの配線はありません。
* データが古いときは読み取り専用になり、そのことを表示します。

## 🧩 A.R.M.O.R. での役割

[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR) が機器を読み取り、[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER) がその内容を保持してすべての命令を判断し、この画面が表示します。[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI) はシステム全体のパネルで、こちらはエネルギー専用です。

## 🏛️ アーキテクチャ

<p align="center">
  <img src="images/architecture.svg" alt="ARMOR-NODE-SOLAR-HMI の構成：シミュレータ、テスト済みコア、サーバー" width="100%">
</p>

## ✨ 機能

* 欠けている値やあり得ない値は表示せず、ゼロにもしません。
* セル、モジュール、スタックにはアイコンと言葉付きのレベルがあり、点滅するのは警報のみで毎秒 2 回までです。
* PIN はソルト付きハッシュで保存し、ロックは次第に長くなり、セッションは短時間です。
* バッテリー操作は、サーバーが許可し、データが新しく、PIN が開かれ、本人が確認した場合のみ提示します。
* 5 つのシナリオを持つシミュレータ。メッセージスキーマと照合します。

## 🔒 セキュリティと安全

| 脅威 | 防御 |
|---|---|
| PIN を推測される | 再起動後も続く段階的なロックと、操作がないと終わる短いセッション |
| 古いデータで操作する | 新しい応答がなければ画面は読み取り専用で、操作を提示しません |
| フラッシュを読み出される | 短い PIN では防げません。基板のフラッシュ暗号化が必要で、命令の判断は常にサーバーが行います |

## 🚀 クイックスタート

**要件：** CMake 3.16 以降、C++17 コンパイラ、Python 3.11 以降。

```powershell
python tools/armor_project_tool.py build-test .  # ホストテスト（CMake と C++17 コンパイラが必要）
build/host/solar_sim cell-high                   # シミュレータの応答を 1 件
```

## 🧪 テスト

```powershell
python tools/armor_project_tool.py build-test .
python -I tools/armor_ci_validate.py
```

| ファイル | 対象 |
|---|---|
| `test_solar_data.cpp` | 読む値と除外する値 |
| `test_solar_view.cpp` | 古いデータ、レベル、点滅 |
| `test_pin_gate.cpp` | PIN のハッシュ、ロック、セッション |
| `test_write_gate.cpp` | 操作を止めるすべての理由 |
| `test_plant.cpp` | 5 つのシナリオ |
| `check_simulator.py` | シミュレータがスキーマに従う |
| `check_base_copies.py` | 共有ファイルが元と一致 |

## 🏗️ ビルドとリリース

ビルドするファームウェアもリリースもまだありません。用意できたら GitHub Actions のみでビルドします。

## 📂 リポジトリの構成

<p align="center">
  <img src="images/repository-structure.svg" alt="ARMOR-NODE-SOLAR-HMI のリポジトリ構造" width="100%">
</p>

<details>
<summary>ツリーのテキスト版</summary>

```text
ARMOR-NODE-SOLAR-HMI/
├── core/
│   ├── solar_data.hpp    サーバーから読む内容
│   ├── solar_view.hpp    古いデータ、レベル、点滅
│   ├── pin_gate.hpp      画面の PIN
│   ├── write_gate.hpp    電池操作を提示する条件
│   ├── json.hpp          共通ファームウェア基盤から
│   └── hmac_sha256.hpp   共通ファームウェア基盤から
├── simulator/
│   ├── plant.hpp         5 つのシナリオ
│   └── main.cpp          応答を出力
├── tests/                ホストテストと契約の検査
├── docs/                 設計と図の元データ
├── tools/                共通の検査
├── images/               README の図
├── .github/              ワークフローとフォーム
├── armor.project.json    マニフェスト
├── CHANGELOG.md          変更履歴
├── LICENSE               GPL-3.0-or-later
├── CONTRIBUTING.md       貢献方法
├── CODE_OF_CONDUCT.md    行動規範
├── SECURITY.md           脆弱性の報告
└── SUPPORT.md            サポート
```

</details>

## 🗺️ ロードマップと既知の制限

**現時点の既知の制限**

* **できること**: サーバーの太陽光応答を読み取り、判定し、PIN を守ります（コンピュータ上）。
* **できないこと**: 画面の描画、接続、命令の送信、基板上での動作。

**今後**

* 小型画面向けの簡潔な太陽光応答を ARMOR-SERVER に追加（先に ARMOR-COMMON で定義）。
* 1024 x 600 の画面をコンピュータ上のシミュレータで 8 言語描画。
* ARMOR-NODE-HMI と共有する基板レイヤー。
* 基板での初回起動。

## 🌍 言語

README とテキストは、英語、スペイン語、フランス語、イタリア語、ドイツ語、中国語、日本語、ヨーロッパポルトガル語で提供されています。

## 🔗 関連プロジェクト

**A.R.M.O.R.**（Autonomous Radar & Multimodal Observation Range）は、独立したリポジトリで構成される境界セキュリティシステムです。各リポジトリは独自のバージョン、テスト、README を持ちます。ファミリーは次のとおりです：

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - メッセージのコントラクト、バリデーター、適合ベクトル、生成された型
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - 3 つのレーダーと独自の Web パネルを備えた ESP32-S3 向けフィールドノードのファームウェア
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - ソーラーインバーターとバッテリーのプロトコル、ゲートウェイノードのメッセージ
* **[ARMOR-NODE-ELECTRICAL](https://github.com/JuanenRac/ARMOR-NODE-ELECTRICAL)** - 電気ノード：メーター、系統の読み取り値のメッセージ、切り替えのルール
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - 警報ノードとパネル：ゾーン、警戒、遅延、サイレン、PIN。サーバーありでもなしでも動作
* **[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI)** - タッチパネル：壁面画面でのシステムの状態、警戒と確認応答、そして音声アシスタントの置き場
* **ARMOR-NODE-SOLAR-HMI**（このリポジトリ） - 太陽光タッチ画面：ARMOR-SERVER が把握するバッテリーとインバーター（初期段階：ファームウェアは未作成）
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - ローカルネットワーク：そのデバイス、インターネット、そして変化
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - 中央コーディネーター：テレメトリ、警報、デバイス、ソーラーの読み取り値、カメラ
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Web コンソール：カメラ、レーダー、警報、太陽光エネルギー、2D/3D のサイトデザイナー
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - ライブの 2D/3D レーダーを備えた Android オペレータークライアント
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - iPhone オペレータークライアント
* **[ARMOR-WATCH](https://github.com/JuanenRac/ARMOR-WATCH)** - 最小のウォッチクライアント：アラーム通知とアラームの確認
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - 判断を説明し、決して作動させない視覚推論ポリシー
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Jetson Orin NX 向けのカメラゾーン、マスク、検出ルール
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - 偽造できない確認つきのオフライン音声インテント
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - 筐体、電子回路、ベンチ受け入れマトリクス
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - デプロイ、CM5 テストベンチ、バックアップ、TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - 再現可能な障害を持つオフラインのテレメトリシミュレーター
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - エコシステム自身のリポジトリを検出、インストール、更新
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - アーキテクチャ、セキュリティのベースライン、機能マトリクス

## 📚 ドキュメント

* [設計](docs/DESIGN.md)：ルールと計画
* [機能マトリクス：何が証明され、何が証明されていないか](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [プロジェクトカタログ：バージョンと、リポジトリ同士の依存関係](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [このリポジトリの変更履歴](CHANGELOG.md)

## 🤝 コントリビュートとコミュニティ

* プルリクエストの前に `python tools/armor_project_tool.py build-test .` と `python -I tools/armor_ci_validate.py` を実行してください。両方の成功が必要です。[CONTRIBUTING.md](CONTRIBUTING.md) と[行動規範](CODE_OF_CONDUCT.md)を参照してください。
* README は 8 言語で提供され、ARMOR-DOCS のソースから生成されます。文面の変更はそちらで行います。
* コンポーネント間のメッセージの変更は ARMOR-COMMON から始めます。
* ビルドしただけのものを、ボード上で動いているかのように書かないでください。
* 脆弱性は非公開で報告してください：[SECURITY.md](SECURITY.md)。質問、アイデア、報告：electrohobby3d@gmail.com（[SUPPORT.md](SUPPORT.md)）。

## 👤 作者

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 ライセンス

GPL-3.0-or-later - [LICENSE](LICENSE) を参照してください。
