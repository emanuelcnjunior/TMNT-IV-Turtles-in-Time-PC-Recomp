"""Package a playable Linux binary, assets and license notices; never the ROM."""
from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import struct
import subprocess
import sys
import tarfile
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
EXE = "TmntIvTurtlesInTimeSNESRecomp"


def digest(path: Path) -> str:
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build-linux-game")
    parser.add_argument("--sdl-license", type=Path, default=ROOT / "build-linux-tools/linux/deps/share/licenses/SDL3/LICENSE.txt")
    args = parser.parse_args()
    build = args.build_dir.resolve()
    cache = (build / "CMakeCache.txt").read_text(encoding="utf-8")
    for required in ("CMAKE_BUILD_TYPE:STRING=Release", "SNESRECOMP_SETUP_HOST:BOOL=OFF"):
        if required not in cache:
            raise SystemExit(f"Playable Release build required: {required}")
    binary = build / EXE
    header = binary.read_bytes()[:20]
    if header[:6] != b"\x7fELF\x02\x01" or struct.unpack_from("<H", header, 18)[0] != 62:
        raise SystemExit("Expected a Linux x86-64 ELF executable")
    version = (ROOT / "VERSION").read_text().strip()
    name = f"TMNT-IV-Turtles-in-Time-PC-v{version}-linux-x86_64"
    dist = ROOT / "dist"
    dist.mkdir(exist_ok=True)
    identity = subprocess.check_output(
        [sys.executable, str(ROOT / "snesrecomp/tools/rom_identity.py"),
         str(ROOT / "rom_identity.txt"), "--get", "expected_sha256"], text=True).strip()
    with tempfile.TemporaryDirectory(prefix="linux-package-", dir=dist) as temporary:
        stage = Path(temporary) / name
        stage.mkdir()
        shutil.copy2(binary, stage / EXE)
        for directory in ("assets", "mods"):
            shutil.copytree(build / directory, stage / directory)
        for resource in ("fonts/LatoLatin-Regular.ttf", "fonts/LatoLatin-Bold.ttf",
                         "fonts/NotoSansSymbols2-Regular.ttf", "fonts/OpenMoji-black-glyf.ttf",
                         "img/boxart.tga", "img/brand_mark.tga", "img/pad.tga", "img/flags.png",
                         "img/verdict_ok.tga", "img/verdict_bad.tga", "img/verdict_none.tga", "img/verdict_warn.tga"):
            if not (stage / "assets" / resource).is_file():
                raise SystemExit(f"Missing launcher asset: {resource}")
        licenses = stage / "licenses"
        licenses.mkdir()
        notices = {
            ROOT / "LICENSE": "project.txt",
            ROOT / "snesrecomp/LICENSE": "framework.txt",
            ROOT / "snesrecomp/THIRD_PARTY_ATTRIBUTION.md": "framework-third-party.md",
            ROOT / "recomp-ui/LICENSE": "launcher.txt",
            ROOT / "recomp-ui/src/third_party/imgui/LICENSE.txt": "imgui.txt",
            ROOT / "recomp-ui/assets/common/fonts/NOTICE.md": "fonts.md",
            ROOT / "recomp-ui/assets/common/img/NOTICE.md": "images.md",
            ROOT / "launcher_assets/img/BOXART_SOURCE.txt": "boxart.txt",
            args.sdl_license: "SDL3.txt",
        }
        for source, destination in notices.items():
            shutil.copy2(source, licenses / destination)
        # These single-file libraries carry their complete license in the header.
        for filename in ("stb_image.h", "stb_image_write.h", "stb_truetype.h", "tinyfiledialogs.h"):
            text = (ROOT / "recomp-ui/src/third_party" / filename).read_text(encoding="utf-8")
            # Retain the embedded notices verbatim without shipping implementation.
            if filename.startswith("stb_"):
                marker = text.rfind("ALTERNATIVE A")
                if marker < 0:
                    raise SystemExit(f"Missing embedded license: {filename}")
                text = text[marker:]
            else:
                end = text.find("*/")
                if end < 0:
                    raise SystemExit(f"Missing header notice: {filename}")
                text = text[:end + 2]
            (licenses / (filename + ".txt")).write_text(text, encoding="utf-8")
        (stage / "run.sh").write_text(
            '#!/bin/sh\nset -eu\ncd -- "$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"\n'
            f'exec ./{EXE} "$@"\n', encoding="utf-8", newline="\n")
        (stage / "LEIA-ME.txt").write_text(
            f"TMNT IV PC Recomp {version} - Linux x86-64 (experimental)\n\n"
            "Extraia toda a pasta e execute ./run.sh. No launcher, selecione sua ROM USA.\n"
            "Nao e necessario recompilar o jogo nem instalar ferramentas de desenvolvimento.\n"
            "A ROM nao esta incluida. Use sua propria copia compativel.\n"
            "Requer Linux x86-64 com glibc 2.35+, libstdc++ (GLIBCXX_3.4.29+) e OpenGL.\n"
            "Ubuntu 22.04 ou mais recente e a base de compatibilidade.\n"
            "Configuracoes e saves ficam na pasta extraida. Mantenha os arquivos juntos.\n"
            "F1/F2: Quick Save/Load. F11: saves. F6: Original/HD. F10: graficos.\n"
            "A campanha, audio e controles ainda precisam de validacao manual no Linux.\n"
            "Nao presuma compatibilidade entre save states Windows e Linux.\n",
            encoding="utf-8")
        revision = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip()
        pins = subprocess.check_output(["git", "submodule", "status", "--recursive"], cwd=ROOT, text=True)
        (stage / "BUILD-INFO.txt").write_text(
            f"version={version}\ngame={revision}\nplatform=linux-x86_64\n"
            "compiler=Clang 22.1.8\ntoolchains=v1.0.14\nsetup_host=OFF\nframe_impl=LLE\n" + pins,
            encoding="utf-8")
        forbidden = {".sfc", ".smc", ".rom", ".bin", ".srm", ".state", ".c", ".cpp", ".h", ".py", ".rs", ".o", ".obj", ".a"}
        manifest = []
        for path in sorted(stage.rglob("*")):
            if not path.is_file():
                continue
            sha = digest(path)
            if path.suffix.lower() in forbidden or path.name in {"rom.cfg", "state.toml", "CMakeCache.txt"} or sha == identity:
                raise SystemExit(f"Forbidden ROM, source or user data in package: {path.relative_to(stage)}")
            manifest.append({"path": path.relative_to(stage).as_posix(), "bytes": path.stat().st_size, "sha256": sha})
        (stage / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

        def permissions(info: tarfile.TarInfo) -> tarfile.TarInfo:
            info.uid = info.gid = 0
            info.uname = info.gname = "root"
            info.mode = 0o755 if info.isdir() or Path(info.name).name in {EXE, "run.sh"} else 0o644
            return info

        archive = dist / (name + ".tar.gz")
        with tarfile.open(archive, "w:gz") as tar:
            tar.add(stage, arcname=name, filter=permissions)
    archive.with_suffix(archive.suffix + ".sha256").write_text(f"{digest(archive)}  {archive.name}\n", encoding="ascii")
    print(archive)


if __name__ == "__main__":
    main()
