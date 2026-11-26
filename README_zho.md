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
  🇨🇳 <b>简体中文</b> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### 家中的能源屏幕：一块 7 英寸触摸屏，将按照 ARMOR-SERVER 掌握的情况显示电池组、电芯和逆变器

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 or later">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Language">
  <img src="https://img.shields.io/badge/Board-ESP32--P4%207%20inch%20(planned)-e7352c.svg" alt="Board">
  <img src="https://img.shields.io/badge/Tests-5%20host%20programs-2ea44f.svg" alt="Tests">
  <img src="https://img.shields.io/badge/Version-0.0.1-0f6cbd.svg" alt="Version">
  <img src="https://img.shields.io/badge/Maturity-scaffolding-FFB020.svg" alt="Maturity">
</p>

---

**诚实性检查 - 今天真正能运行的部分：** 屏幕背后的逻辑和一个设备模拟器已经编写完成，并在电脑上测试（5 个主机程序）。**目前还没有固件、界面，也没有与服务器的连接；这里没有任何东西向电池发送指令，也没有任何内容在电路板上运行过。**

---

## 📋 速览

| | |
|---|---|
| 版本 | `0.0.1` |
| 成熟度 | 雏形 |
| 角色 | A.R.M.O.R. 的太阳能触摸屏（家族：Clients，上级：ARMOR-SERVER） |
| 技术栈 | C++17 核心与模拟器、CMake；Python 检查 |
| 测试 | 5 个主机程序 |
| 电路板 | Guition JC1060P470C（ESP32-P4 搭配 ESP32-C6，7 英寸 1024 x 600）；尚无板级代码 |
| 运行环境 | 装有 CMake、C++17 编译器和 Python 3.11+ 的电脑 |
| 真实硬件上 | 电路板上尚未运行任何内容 |
| 许可证 | GPL-3.0-or-later |

**跳转到：** [概述](#-概述) · [架构](#-架构) · [功能](#-功能) · [安全](#-安全与防护) · [快速开始](#-快速开始) · [测试](#-测试) · [结构](#-仓库结构) · [相关项目](#-相关项目)

## 🚦 项目状态

每一行是本仓库的一个部分，灯号表示它今天做到了哪一步。内容依据仓库能够展示的证据（测试、构建、运行），而不是计划。

🟢 已完成并验证 · 🟡 可用，有已知限制 · 🟠 部分完成或尚未验证 · 🔴 未完成或不可用

| 领域 | 状态 | 在这里的含义 |
|---|---|---|
| 软件 | 🟠 | 负责读取、判断和保护的核心已编写；固件尚未编写。 |
| 测试 | 🟡 | 主机程序以及两项对照 ARMOR-COMMON 的检查；处于早期阶段。 |
| 界面 | 🔴 | 尚未绘制任何界面。 |
| 硬件 | 🔴 | 电路板没有代码，也没有运行过任何内容。 |
| 安全 | 🟠 | PIN 与写入检查仅是经过测试的逻辑；没有独立审查。 |
| 更新 | 🔴 | 尚无可更新的内容。 |
| 集成 | 🔴 | 尚未连接 ARMOR-SERVER；所需的精简太阳能接口还不存在。 |
| 文档 | 🟡 | 八种语言的 README 和设计说明。 |

## 🎯 概述

* 显示设备的能源部分：总量、电池组、电芯和逆变器。
* 全部通过网络从服务器读取；没有串口线或电池线。
* 数据过旧时变为只读，并明确提示。

## 🧩 在 A.R.M.O.R. 中的角色

[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR) 读取设备，[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER) 保存其数据并决定每一条指令，本屏幕负责显示。[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI) 是整个系统的面板；本屏幕只关注能源。

## 🏛️ 架构

<p align="center">
  <img src="images/architecture.svg" alt="ARMOR-NODE-SOLAR-HMI 架构：模拟器、经过测试的核心和服务器" width="100%">
</p>

## ✨ 功能

* 缺失或不可能的数值不显示，也绝不显示为零。
* 电芯、模块和电池组的级别带有图标和文字；只有警报闪烁，每秒最多两次。
* PIN 以加盐哈希保存，锁定时间递增，会话很短。
* 仅当服务器允许、数据新鲜、PIN 已解锁且本人确认时，才提供电池操作。
* 包含五个场景的模拟器，并对照消息模式进行检查。

## 🔒 安全与防护

| 威胁 | 防御 |
|---|---|
| 有人猜测 PIN | 锁定时间递增且重启后仍有效；会话很短，无触摸即结束 |
| 依据过旧数据操作 | 没有最新应答时屏幕只读，不提供任何操作 |
| 闪存被读取 | 短 PIN 无法抵御：电路板需要加密闪存；每条指令仍由服务器决定 |

## 🚀 快速开始

**要求：** CMake 3.16+、C++17 编译器和 Python 3.11+。

```powershell
python tools/armor_project_tool.py build-test .  # 主机测试（需要 CMake 和 C++17 编译器）
build/host/solar_sim cell-high                   # 模拟器的一条应答
```

## 🧪 测试

```powershell
python tools/armor_project_tool.py build-test .
python -I tools/armor_ci_validate.py
```

| 文件 | 覆盖内容 |
|---|---|
| `test_solar_data.cpp` | 读取与忽略的内容 |
| `test_solar_view.cpp` | 旧数据、级别、闪烁 |
| `test_pin_gate.cpp` | PIN 哈希、锁定、会话 |
| `test_write_gate.cpp` | 阻止操作的每一种原因 |
| `test_plant.cpp` | 五个场景 |
| `check_simulator.py` | 模拟器符合模式 |
| `check_base_copies.py` | 共享文件与来源一致 |

## 🏗️ 构建与发布

目前还没有可编译的固件，也没有发布版本。将来只在 GitHub Actions 中编译。

## 📂 仓库结构

<p align="center">
  <img src="images/repository-structure.svg" alt="ARMOR-NODE-SOLAR-HMI 的仓库结构" width="100%">
</p>

<details>
<summary>目录树的文本版本</summary>

```text
ARMOR-NODE-SOLAR-HMI/
├── core/
│   ├── solar_data.hpp    从服务器读取的内容
│   ├── solar_view.hpp    旧数据、级别、闪烁
│   ├── pin_gate.hpp      屏幕的 PIN
│   ├── write_gate.hpp    何时提供电池操作
│   ├── json.hpp          来自共享固件基础
│   └── hmac_sha256.hpp   来自共享固件基础
├── simulator/
│   ├── plant.hpp         五个场景
│   └── main.cpp          打印应答
├── tests/                主机测试与契约检查
├── docs/                 设计与图片源文件
├── tools/                共享检查
├── images/               README 图片
├── .github/              工作流与表单
├── armor.project.json    清单
├── CHANGELOG.md          变更记录
├── LICENSE               GPL-3.0-or-later
├── CONTRIBUTING.md       如何贡献
├── CODE_OF_CONDUCT.md    行为准则
├── SECURITY.md           漏洞报告
└── SUPPORT.md            获取帮助
```

</details>

## 🗺️ 路线图与已知限制

**当前已知限制**

* **它做什么**: 在电脑上读取服务器的太阳能应答、进行判断并保护 PIN。
* **它不做什么**: 绘制界面、建立连接、发送指令或在电路板上运行。

**下一步**

* 在 ARMOR-SERVER 中为小屏幕提供精简的太阳能应答，先在 ARMOR-COMMON 中定义。
* 用电脑上的模拟器以八种语言绘制 1024 x 600 界面。
* 与 ARMOR-NODE-HMI 共享的板级层。
* 在电路板上首次上电。

## 🌍 语言

README 和文本提供英语、西班牙语、法语、意大利语、德语、汉语、日语和欧洲葡萄牙语版本。

## 🔗 相关项目

**A.R.M.O.R.**（Autonomous Radar & Multimodal Observation Range）是一个由相互独立的仓库组成的周界安全系统。每个仓库都有自己的版本、自己的测试和自己的 README；这是整个系列：

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - 消息契约、验证器、一致性向量和生成的类型
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - 面向 ESP32-S3 的现场节点固件，配三个雷达和自带的网页面板
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - 太阳能逆变器和电池的协议，以及网关节点的消息
* **[ARMOR-NODE-ELECTRICAL](https://github.com/JuanenRac/ARMOR-NODE-ELECTRICAL)** - 电气节点：电表、电网读数的消息和切换规则
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - 报警节点与面板：防区、布防、延时、警笛和 PIN，可配合服务器也可独立使用
* **[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI)** - 触摸面板：壁挂屏幕上的系统状态、布防与确认，以及语音助手的所在
* **ARMOR-NODE-SOLAR-HMI**（本仓库） - 太阳能触摸屏：按 ARMOR-SERVER 掌握的情况显示电池和逆变器（早期阶段：尚无固件）
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - 本地网络：其设备、互联网以及变化
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - 中央协调器：遥测、报警、设备、太阳能读数和摄像头
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - 网页控制台：摄像头、雷达、报警、太阳能以及 2D/3D 场地设计器
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - 带实时 2D/3D 雷达的 Android 操作员客户端
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - iPhone 操作员客户端
* **[ARMOR-WATCH](https://github.com/JuanenRac/ARMOR-WATCH)** - 极简手表客户端：警报通知与警报确认
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - 视觉推理策略，会解释其决策且从不驱动设备
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - 面向 Jetson Orin NX 的摄像头区域、遮罩和检测规则
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - 离线语音意图，附带无法伪造的确认
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - 外壳、电子器件和台架验收矩阵
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - 部署、CM5 测试台、备份和 TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - 带可重复故障的离线遥测模拟器
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - 检测、安装和更新生态系统自身的仓库
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - 架构、安全基线和能力矩阵

## 📚 文档

* [设计](docs/DESIGN.md)：规则与计划
* [能力矩阵：哪些已被证明，哪些没有](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [项目目录：版本以及各仓库之间如何相互依赖](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [本仓库的更新日志](CHANGELOG.md)

## 🤝 参与贡献与社区

* 提交拉取请求前请运行 `python tools/armor_project_tool.py build-test .` 和 `python -I tools/armor_ci_validate.py`：两者都必须通过。参见 [CONTRIBUTING.md](CONTRIBUTING.md) 和[行为准则](CODE_OF_CONDUCT.md)。
* README 提供八种语言，由 ARMOR-DOCS 中的源文件生成：文本的修改在那里进行。
* 组件之间消息的变更从 ARMOR-COMMON 开始。
* 不要把仅仅构建过的东西描述为已在板子上运行。
* 请私下报告漏洞：[SECURITY.md](SECURITY.md)。问题、想法和报告：electrohobby3d@gmail.com（[SUPPORT.md](SUPPORT.md)）。

## 👤 作者

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 许可证

GPL-3.0-or-later - 见 [LICENSE](LICENSE)。
