#!/usr/bin/env python3
#
#-- Version Date: 10-02-2026 -- (dd-mm-eyy)
#
import subprocess
import sys
from pathlib import Path

# ---------------- CONFIG ----------------

# Extensions to format
EXTENSIONS = {".c", ".cpp", ".h"}

# Directories to skip (relative path fragments)
SKIP_DIRS = {
    ".git",
    ".pio",
    ".pio.nosync",
    "managed_components",
    "build",
    "sdkconfig",
    "sdkconfig.defaults",

    # ESP-IDF / third party
    "components/esp-idf",
    "components/lwip",
    "components/mdns",
    "components/freertos",
}

# ----------------------------------------


def clang_format_exists():
    try:
        subprocess.run(
            ["clang-format", "--version"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=True,
        )
        return True
    except Exception:
        return False


def should_skip(path: Path) -> bool:
    for part in path.parts:
        for skip in SKIP_DIRS:
            if skip in path.as_posix():
                return True
    return False


def format_file(path: Path):
    try:
        subprocess.run(
            ["clang-format", "-i", str(path)],
            check=True,
        )
        print(f"✔ formatted {path}")
    except subprocess.CalledProcessError as e:
        print(f"✖ failed    {path}: {e}")


def main():
    if not clang_format_exists():
        print("❌ clang-format not found")
        print("Install it first:")
        print("  macOS:  brew install clang-format")
        print("  Linux:  sudo apt install clang-format")
        sys.exit(1)

    root = Path(".").resolve()
    files = []

    for path in root.rglob("*"):
        if not path.is_file():
            continue

        if path.suffix not in EXTENSIONS:
            continue

        if should_skip(path):
            continue

        files.append(path)

    if not files:
        print("Nothing to format.")
        return

    print(f"Formatting {len(files)} files...\n")

    for f in files:
        format_file(f)

    print("\n✅ Done.")


if __name__ == "__main__":
    main()
