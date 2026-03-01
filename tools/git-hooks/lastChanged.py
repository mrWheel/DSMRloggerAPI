#!/usr/bin/env python3
#
#-- Version Date: 10-02-2026 -- (dd-mm-eeyy)
#
"""
===============================================================================
Git pre-commit hook helper: lastChanged.py
===============================================================================

This script updates (or inserts) a timestamp header at the top of staged
C/C++ source and header files, in the form:

    /*** Last Changed: YYYY-MM-DD - HH:MM ***/

It is intended to be executed automatically as a Git *pre-commit hook*.
Running it manually is possible, but it will only act on *staged* files.

-------------------------------------------------------------------------------
INSTALLATION (required once per clone)
-------------------------------------------------------------------------------

Git does NOT automatically run scripts in this directory.
You must explicitly tell Git to use this folder for hooks.

From the repository root, run:

    git config core.hooksPath tools/git-hooks

This config is local to the repository and must be set per clone.

-------------------------------------------------------------------------------
PRE-COMMIT HOOK SETUP
-------------------------------------------------------------------------------

Ensure there is a file named exactly 'pre-commit' (no extension) in this
directory, with the following contents:

    #!/bin/sh
    python3 tools/git-hooks/last-changed.py

And make it executable:

    chmod +x tools/git-hooks/pre-commit
    chmod +x tools/git-hooks/last-changed.py

-------------------------------------------------------------------------------
HOW IT WORKS
-------------------------------------------------------------------------------

- Git stages files first (via CLI or GUI like VSCode / PlatformIO)
- Git runs the pre-commit hook
- This script:
    - inspects ONLY staged files
    - updates matching .cpp/.h files
    - re-stages modified files automatically

If no staged files are found, the script exits quietly.

-------------------------------------------------------------------------------
NOTES
-------------------------------------------------------------------------------

- This script is designed to run during 'git commit'
- It will not modify unstaged files
- GUI commits (VSCode, PlatformIO) work as long as hooks are enabled
- Hooks are skipped if Git is invoked with '--no-verify'

===============================================================================
"""

import subprocess
import datetime
import sys
import re
from pathlib import Path
from typing import Optional, List

print("HOOK RUNNING (pre-commit)", file=sys.stderr, flush=True)

# =========================
# Config
# =========================

# Mappen waarin we staged *.cpp willen updaten
CPP_DIRS = [
    "src",
    # "lib",
    # "components",
]

# Mappen waarin we staged *.h willen updaten
# (headers kunnen ook in src/lib staan)
H_DIRS = [
    "include",
    "src",
    # "lib",
]

# Propagate-regels:
# Als een staged gewijzigde .cpp onder CPP_DIRS valt, dan proberen we de bijbehorende .h
# te updaten door hetzelfde relatieve pad te nemen voor het bijbehorende .h bestand,
# onder deze header-roots:
PROPAGATE_HEADER_DIRS = [
    "include",
    # "lib/somelib/include",
]

def make_header(now_str: str) -> str:
    return f"/*** Last Changed: {now_str} ***/\n"

# Match alleen als de header echt bovenaan staat (optioneel met UTF-8 BOM)
HEADER_AT_TOP_RE = re.compile(r"^(?:\ufeff)?/\*\*\*\s*Last Changed: .*?\*\*\*/\s*\n", re.DOTALL)

# =========================
# Git helpers
# =========================

def run_git(args: List[str]) -> subprocess.CompletedProcess:
    return subprocess.run(["git", *args], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

def get_repo_root() -> Path:
    res = run_git(["rev-parse", "--show-toplevel"])
    if res.returncode != 0:
        print(res.stderr.strip())
        raise SystemExit(1)
    return Path(res.stdout.strip())

# =========================
# Path helpers
# =========================

def is_under_any_dir(rel_path: Path, dir_list: List[str]) -> bool:
    s = rel_path.as_posix()
    return any(s == d or s.startswith(d.rstrip("/") + "/") for d in dir_list)

def rel_under_root(rel_path: Path, roots: List[str]) -> Optional[Path]:
    """
    Als rel_path onder een van de roots valt, return het pad relatief t.o.v. die root.
    Bijvoorbeeld: rel_path='src/foo/a.cpp' en root='src' => 'foo/a.cpp'
    """
    s = rel_path.as_posix()
    for r in roots:
        r = r.rstrip("/")
        if s == r:
            return Path(".")
        if s.startswith(r + "/"):
            return Path(s[len(r) + 1 :])
    return None

def dedup_paths(paths: List[Path]) -> List[Path]:
    seen = set()
    out = []
    for p in paths:
        sp = p.as_posix()
        if sp not in seen:
            seen.add(sp)
            out.append(p)
    return out

# =========================
# File update
# =========================

def update_file_header(abs_path: Path, header_line: str) -> bool:
    """
    Zet of vervang de Last Changed header bovenaan het bestand.
    Return True als het bestand is aangepast.
    """
    if not abs_path.exists() or not abs_path.is_file():
        return False

    original = abs_path.read_text(encoding="utf-8", errors="replace")

    # behoud BOM als die er is
    bom = "\ufeff" if original.startswith("\ufeff") else ""
    content = original[len(bom):] if bom else original

    if HEADER_AT_TOP_RE.match(original):
        new_text = HEADER_AT_TOP_RE.sub(bom + header_line, original, count=1)
    else:
        new_text = bom + header_line + content

    if new_text != original:
        abs_path.write_text(new_text, encoding="utf-8", newline="\n")
        return True
    return False

# =========================
# Main
# =========================

repo_root = get_repo_root()

# Staged files (Added/Copied/Modified)
res = run_git(["diff", "--cached", "--name-only", "--diff-filter=ACM"])
if res.returncode != 0:
    print(res.stderr.strip())
    raise SystemExit(1)

staged_rel = [Path(p) for p in res.stdout.splitlines() if p.strip()]

# 🔒 Guard: niets staged → waarschijnlijk handmatig gerund
if not staged_rel:
    print("last-changed: no staged files (this hook runs during commit)", file=sys.stderr)
    raise SystemExit(0)

# Selecteer staged cpp/h op basis van config mappen
staged_cpp_rel = [
    p for p in staged_rel
    if p.suffix == ".cpp" and is_under_any_dir(p, CPP_DIRS)
]

staged_h_rel = [
    p for p in staged_rel
    if p.suffix == ".h" and is_under_any_dir(p, H_DIRS)
]

# Propagate: voor elke staged gewijzigde cpp -> include-headers met dezelfde folderstructuur
propagated_h_rel: List[Path] = []

for cpp_rel in staged_cpp_rel:
    rel_inside_cpp_root = rel_under_root(cpp_rel, CPP_DIRS)
    if rel_inside_cpp_root is None:
        continue

    # vervang extensie naar .h, behoud subpad
    rel_h_inside = rel_inside_cpp_root.with_suffix(".h")

    # probeer exact pad onder elke propagate header root
    for hdr_root in PROPAGATE_HEADER_DIRS:
        candidate_rel = Path(hdr_root) / rel_h_inside
        candidate_abs = repo_root / candidate_rel
        if candidate_abs.exists():
            propagated_h_rel.append(candidate_rel)

# Dedup
targets_rel = dedup_paths(staged_cpp_rel + staged_h_rel + dedup_paths(propagated_h_rel))

# Timestamp (lokale tijd)
now_str = datetime.datetime.now().strftime("%Y-%m-%d - %H:%M")
header_line = make_header(now_str)

# Update en stage
for rel_path in targets_rel:
    abs_path = repo_root / rel_path
    if update_file_header(abs_path, header_line):
        add_res = run_git(["add", rel_path.as_posix()])
        if add_res.returncode != 0:
            print(add_res.stderr.strip())
            raise SystemExit(1)

raise SystemExit(0)
