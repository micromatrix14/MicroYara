# MicroYara

A lightweight IDE for writing, compiling, and testing [YARA](https://virustotal.github.io/yara/) rules — built for malware researchers and security analysts.

![License](https://img.shields.io/badge/license-AGPL--3.0-blue)
![Language](https://img.shields.io/badge/C%2B%2B-17-blue)
![Framework](https://img.shields.io/badge/Qt-5%20%7C%206-green)

---

## Features

- **Syntax Highlighting** — Color-coded YARA keywords, strings, regex patterns, and comments
- **Auto-Completion** — Intelligent suggestions for 47+ YARA keywords as you type
- **Live Compilation** — Automatic compilation after a configurable timeout (1–10 seconds)
- **Integrated Scanning** — Load files or folders and scan them with your compiled rules
- **Drag & Drop** — Drop files/folders directly onto the app to load scan targets
- **Dark Theme** — Easy on the eyes with a carefully chosen color palette
- **Customizable Arguments** — Pass custom flags to both the compiler and scanner
- **Namespace Support** — Organize rules with optional namespaces

## Screenshot

<!-- Add a screenshot here: ![MicroYara](screenshot.png) -->

## Prerequisites

- **Qt 5 or Qt 6** (with Widgets module)
- **YARA binaries** — Place `yarac64.exe` and `yara64.exe` in the application directory
  - Download from [VirusTotal/yara releases](https://github.com/VirusTotal/yara/releases)

## Building

```bash
# Using qmake
qmake MicroYara.pro
make        # or nmake / mingw32-make on Windows

# Or open MicroYara.pro in Qt Creator and build from there
```

## Usage

1. **Write** YARA rules in the code editor (left panel). Auto-completion kicks in after 2 characters.
2. **Compile** — Rules auto-compile after the configured timeout, or click the Build button (hammer icon) to compile manually.
3. **Load Target** — Click "Load File/Folder" or drag & drop a file/folder onto the button.
4. **Scan** — Enable the "Auto-Scan" checkbox to scan automatically after each successful compilation, or trigger scans manually.
5. **Review** — Check compilation status (green = success, red = failure) and scan results in the output panel.

### Compilation Options

| Field | Description |
|-------|-------------|
| **Timeout** | Seconds to wait after last edit before auto-compiling (1–10) |
| **Namespace** | Optional namespace for compiled rules |
| **Compilation Args** | Extra flags for `yarac64.exe` (e.g., `--fail-on-warnings`) |

### Scanning Options

| Field | Description |
|-------|-------------|
| **Scanning Args** | Extra flags for `yara64.exe` (e.g., `-s` for strings, `-r` for recursive) |
| **Auto-Scan** | Automatically scan after successful compilation |

## Project Structure

```
MicroYara/
├── main.cpp                # Application entry point
├── widget.cpp/h            # Main window logic (compilation, scanning, UI)
├── widget.ui               # Qt Designer UI layout
├── codeeditor.cpp/h        # Custom code editor with line numbers
├── yarahighlighter.cpp/h   # YARA syntax highlighting
├── yaracompleter.cpp/h     # YARA keyword auto-completion
├── MicroYara.pro           # qmake project file
├── resources.qrc           # Icons and resources
└── files/                  # Runtime directory for temp files
```

## Technology Stack

| Component | Technology |
|-----------|-----------|
| Language | C++17 |
| Framework | Qt (Widgets) |
| Build System | qmake |
| External Tools | YARA (`yarac64.exe`, `yara64.exe`) |

## License

This project is licensed under the [GNU Affero General Public License v3.0](LICENSE).
