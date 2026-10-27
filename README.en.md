# TMNT IV: Turtles in Time — PC Recomp

[**🇧🇷 Português**](README.md) | [**🇺🇸 English**](README.en.md)

A Windows port of **Teenage Mutant Ninja Turtles IV: Turtles in Time** (SNES, USA version), built with [SNESRecomp](https://github.com/emanuelcnjunior/snesrecomp-tmnt4). Features **local two-player co-op** and a Windows installer that validates your ROM.

> **ROM not included.** You must provide your own compatible USA copy. This is an unofficial project not affiliated with Konami, Nintendo, or the Teenage Mutant Ninja Turtles rights holders.

## ⬇️ Download and install

**[Download Windows installer — v1.0.2](https://github.com/emanuelcnjunior/TMNT-IV-Turtles-in-Time-PC-Recomp/releases/download/v1.0.2/TMNT-IV-Turtles-in-Time-PC-Setup-v1.0.2.exe)**

**Requirements:** Windows 10 (1903+) or later, **64-bit**, OpenGL-compatible graphics driver, and a valid USA ROM. Players do **not** need Git, Python, Rust, CMake, or Visual Studio.

1. Download and run the installer.
2. When prompted, choose **your own USA ROM**.
3. The installer validates and imports the file into your user data folder.
4. Launch the game from its shortcut and configure video, audio, and controllers in the launcher.

The source filename may differ: **SHA-256 is used to verify the actual contents**, and a valid ROM is stored under its canonical filename. Other regions, modified dumps, and copier-header ROMs are rejected.

[All releases](https://github.com/emanuelcnjunior/TMNT-IV-Turtles-in-Time-PC-Recomp/releases) · [v1.0.2 release notes and checksums](installer/RELEASE-v1.0.2.md) · [Detailed installer guide (Portuguese)](installer/README.md)

## 🎮 Controls and features

| Action | Keyboard | Controller |
|---|---|---|
| Quick Save | **F1** | **LT / L2** |
| Quick Load | **F2** | **RT / R2** |
| Save-state list | **F11** | See launcher mappings |
| Toggle Original / HD | **F6** | **Start** (still acts as Start in-game) |
| Graphics settings | **F10** | Via UI |
| Open launcher | **Ctrl + L** | — |

Keyboard shortcuts can be remapped under **Settings > Hotkeys**. Controller behavior can vary with your mappings.

### 💾 Save games

The port supports **five manual slots** and **one Quick Save slot**. These are *save states* (snapshots of the running game), **not** the original cartridge's SRAM saves.

Save files live at:

```text
%LOCALAPPDATA%\TMNTIVRecomp\saves\
```

Save files include integrity and compatibility checks. **Runtime upgrades may invalidate older states**; back up saves before updating. See the [save-state technical report (Portuguese)](SAVE-GAME-REPORT.md) and [save UI audit](SAVE-UI-AUDIT.md) for test details and limitations.

### 🖼️ Classic and HD modes

The gameplay screenshots show the same stage using two selectable presentations, **without changing the original characters, collisions, or gameplay**:

- **Classic / Original:** retains the SNES pixel-art presentation.
- **HD:** applies GPU upscaling and graphics filters. **Scale2x** refines some pixel edges; it does not replace character sprites or scenery with redrawn artwork.
- **Instant toggle:** press **F6** on the keyboard or **Start** on a controller; Start still works normally in-game.

#### Graphics settings menu (F10)

The captured menu includes **Original/HD mode**, **Scale2x filtering**, **output resolution** (such as 1920 × 1080), **scaling**, **4:3 aspect ratio**, **VSync**, **fullscreen**, **HD Pack**, **reload**, and **ID diagnostics**.

- **4:3:** preserves the traditional SNES aspect ratio.
- **WIDE FILL / PREENCHER:** fills 16:9 screens by stretching the image horizontally. Turn it off for undistorted 4:3.
- **HD Pack:** requires a compatible external pack. The **“MANIFESTO AUSENTE / MISSING MANIFEST”** message shown in the screenshot means **no HD-pack manifest was found**; Original and Scale2x do not require one.
- **Diagnostics / IDs:** developer-oriented graphics resource identification.

**Important:** HD mode currently improves presentation/upscaling; it is **not a complete texture, sprite, or background remaster**. Optional HD packs must use original or appropriately licensed artwork.

### 💾 Save States screen

The captured UI shows **six positions: five manual slots and one Quick Save**, with gameplay thumbnails, save date/time, and empty slots marked **NEW SLOT**. Navigation supports keyboards and controllers.

- **F11:** opens the save-state list.
- **Arrow keys / D-pad:** choose a slot.
- **X (keyboard) or A (controller):** load the selected slot.
- **S (keyboard) or X (controller):** save to the selected slot, following the on-screen legend.
- **Esc (keyboard) or B (controller):** go back.
- **F1 / LT-L2** and **F2 / RT-R2:** quick-save and quick-load respectively.

Save files are stored in `%LOCALAPPDATA%\\TMNTIVRecomp\\saves\\`. Save-state formats may become incompatible after runtime updates; make a backup before upgrading.

## ✅ Project status and testing

The installer and ROM validation are implemented. Project reports cover installation, import, data integrity, boot smoke tests, and dedicated save-state tests. **These do not constitute complete gameplay certification across all hardware.** Some manual and integration checks remain open in the reports.

- [Installer test results](installer/TEST-RESULTS.md)
- [Save-state report (Portuguese)](SAVE-GAME-REPORT.md)
- [v1.0.2 release notes (Portuguese)](installer/RELEASE-v1.0.2.md)

Found a bug? [Open an issue](https://github.com/emanuelcnjunior/TMNT-IV-Turtles-in-Time-PC-Recomp/issues) with your version, Windows build, reproduction steps, and optionally a log **without attaching ROMs or extracted game assets**.

## 🧩 Supported ROM

| Property | Required value |
|---|---|
| Title | Teenage Mutant Ninja Turtles IV: Turtles in Time |
| Region | USA / North America |
| Format | Headerless LoROM, `.sfc` |
| Stored filename | `Teenage Mutant Ninja Turtles IV - Turtles in Time (USA).sfc` |
| File size | 1,048,576 bytes |
| CRC32 | `5940BD99` |
| SHA-256 | `5b82cdd6f2da56f43680d6a5021faebe2e06036d30602c1a7917aa414cf8b5f4` |

The validated ROM is stored in `%LOCALAPPDATA%\TMNTIVRecomp\`. Uninstalling leaves this directory in place, including settings and saves. Manually deleting it **also deletes the imported ROM copy**.

## 🛠️ Build from source

This section is for **developers**, not installer users. It requires the build tools described in the [installer guide (Portuguese)](installer/README.md) and a compatible ROM supplied by the developer.

```bash
git clone --recurse-submodules https://github.com/emanuelcnjunior/TMNT-IV-Turtles-in-Time-PC-Recomp.git
cd TMNT-IV-Turtles-in-Time-PC-Recomp
bash tools/regen.sh --rom "/c/path/to/your-usa-rom.sfc"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
```

ROM regeneration requires Bash; on Windows, compile in an environment configured for MSVC and CMake. Generated `src/gen/` code is not committed, and **ROM files must never be committed**.

For Windows installer packaging, see [installer/README.md](installer/README.md). The project uses the [snesrecomp-tmnt4](https://github.com/emanuelcnjunior/snesrecomp-tmnt4) and [recomp-ui](https://github.com/emanuelcnjunior/recomp-ui) submodules.

### Repository layout

| Path | Purpose |
|---|---|
| `src/` | Port and host integration source code |
| `recomp/` | Recompilation configuration and symbols |
| `src/gen/` | Locally generated ROM-derived code (not committed) |
| `snesrecomp/` | SNESRecomp framework submodule |
| `recomp-ui/` | Launcher submodule |
| `installer/` | Installer scripts, documentation, and tests |
| `tools/regen.sh` | ROM verification and code generation |
| `rom_identity.txt` | Expected ROM identity |

## 📄 License and rights

Project-authored code is governed by [LICENSE](LICENSE). Dependencies and submodules have their own licenses. **The source license does not grant rights to the original game's ROM, characters, artwork, audio, or other assets.** No ROM is provided.
