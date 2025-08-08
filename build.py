import os
import sys
import subprocess
from pathlib import Path
import shutil
import zipfile

# --- Configuration ---------------------------------------------------------

PLATFORMS = {
    "windows": {
        "src_dir": Path("examples/windows"),
        "out_dir": Path("builds/windows"),
        "exe_ext": ".exe",
        "compiler": "g++",
        "base_flags": [
            "-std=c++20",
        ],
        "link_flags": ["-lmmdevapi", "-lole32", "-luuid", "-lksuser", "-lwinmm"],
    },
    "mac": {
        "src_dir": Path("examples/mac"),
        "out_dir": Path("builds/mac"),
        "exe_ext": "",
        "compiler": "g++",
        "base_flags": [
            "-std=c++20",
        ],
        "link_flags": [],
    },
    "linux": {
        "src_dir": Path("examples/linux"),
        "out_dir": Path("builds/linux"),
        "exe_ext": "",
        "compiler": "g++",
        "base_flags": [
            "-std=c++20",
        ],
        "link_flags": [],
    },
}

SRC_CODE_DIR = Path("src")
LIB_DIR = Path("lib")  # optional, only used if present
ZIP_FILE = Path("SoundIO.zip")


# --- Helpers ---------------------------------------------------------------

def detect_available_platforms():
    available = []
    for name, cfg in PLATFORMS.items():
        if cfg["src_dir"].exists() and any(cfg["src_dir"].glob("*.cpp")):
            available.append(name)
    return available

def needs_gui_linking(cpp_path: Path) -> bool:
    """Check if file suggests GUI subsystem."""
    try:
        t = cpp_path.read_text(encoding="utf-8", errors="ignore").lower()
    except Exception:
        return False
    return ("#include <windows.h>" in t) or ("winmain(" in t)

def common_include_flags():
    flags = []
    if SRC_CODE_DIR.exists():
        flags += ["-I", str(SRC_CODE_DIR)]
    if LIB_DIR.exists():
        flags += ["-I", str(LIB_DIR)]
    return flags

def build_platform(platform_key: str, debug_console=True):
    if platform_key not in PLATFORMS:
        print(f"Unknown platform: {platform_key}")
        return

    cfg = PLATFORMS[platform_key]
    src_dir   = cfg["src_dir"]
    out_dir   = cfg["out_dir"]
    exe_ext   = cfg["exe_ext"]
    compiler  = cfg["compiler"]
    base      = list(cfg.get("base_flags", []))
    linkflags = list(cfg.get("link_flags", []))

    if not src_dir.exists():
        print(f"Source directory not found: {src_dir}")
        return

    out_dir.mkdir(parents=True, exist_ok=True)

    cpp_files = sorted(src_dir.glob("*.cpp"))
    if not cpp_files:
        print(f"No .cpp files found in {src_dir}")
        return

    print(f"Building platform: {platform_key}")
    print(f"Source: {src_dir}")
    print(f"Output: {out_dir}\n")

    inc = common_include_flags()

    for cpp in cpp_files:
        target = out_dir / (cpp.stem + exe_ext)

        flags = base.copy()
        if platform_key == "windows":
            if debug_console:
                flags.append("-mconsole")
            elif needs_gui_linking(cpp):
                flags.append("-mwindows")
            else:
                flags.append("-mconsole")

        cmd = [compiler, str(cpp), "-o", str(target)] + flags + inc + linkflags

        print("Compiling:", " ".join(cmd))
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode != 0:
            print(f"Error compiling {cpp.name}:\n{r.stderr.strip()}\n")
        else:
            if r.stdout.strip():
                print(r.stdout.strip())
            print(f"Built {target}\n")

    print(f"All files compiled for '{platform_key}'.\n")


def ensure_miniaudio_in_src():
    """Make sure src/miniaudio.h exists. If not, copy from lib/ if available."""
    ma_in_src = SRC_CODE_DIR / "miniaudio.h"
    if ma_in_src.exists():
        return True
    ma_in_lib = LIB_DIR / "miniaudio.h"
    if ma_in_lib.exists():
        SRC_CODE_DIR.mkdir(parents=True, exist_ok=True)
        shutil.copy2(ma_in_lib, ma_in_src)
        print("Copied 'lib/miniaudio.h' -> 'src/miniaudio.h' for packaging.")
        return True
    print("WARNING: 'src/miniaudio.h' not found and no 'lib/miniaudio.h' to copy from.")
    return False

def pack_source():
    if not SRC_CODE_DIR.exists():
        print(f"Source directory not found: {SRC_CODE_DIR}")
        return
    ensure_miniaudio_in_src()
    if ZIP_FILE.exists():
        ZIP_FILE.unlink()
    with zipfile.ZipFile(ZIP_FILE, "w", zipfile.ZIP_DEFLATED) as zipf:
        for file_path in SRC_CODE_DIR.rglob("*"):
            arcname = file_path.relative_to(SRC_CODE_DIR.parent)
            zipf.write(file_path, arcname)
    print(f"Packed '{SRC_CODE_DIR}' into '{ZIP_FILE}'.\n")

def clean_builds():
    builds_root = Path("builds")
    if builds_root.exists():
        shutil.rmtree(builds_root)
        print(f"Removed '{builds_root}'.\n")
    else:
        print(f"'{builds_root}' does not exist.\n")


# --- Main menu -------------------------------------------------------------

def main():
    print("SoundIO - Building Utility")
    print("Options:")
    print("1. Build examples for a platform (console/debug mode)")
    print("2. Pack for production (src -> SoundIO.zip)")
    print("3. Clean builds")
    choice = input("Select an option: ").strip()

    if choice == "1":
        platforms = detect_available_platforms()
        if not platforms:
            print("No platforms with example .cpp files were found under 'examples/'.")
            return
        print("\nAvailable platforms:")
        for i, name in enumerate(platforms, 1):
            print(f"{i}. {name}")
        sel = input("Select a platform: ").strip()
        try:
            idx = int(sel) - 1
            if idx < 0 or idx >= len(platforms):
                raise ValueError
        except ValueError:
            print("Invalid platform selection.")
            return
        build_platform(platforms[idx], debug_console=True)

    elif choice == "2":
        pack_source()

    elif choice == "3":
        clean_builds()

    else:
        print("Invalid option.")

if __name__ == "__main__":
    main()
