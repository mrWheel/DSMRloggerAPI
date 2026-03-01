#!/usr/bin/env python3
#
#-- Version Date: 25-02-2026 -- (dd-mm-eeyy)
#
#— Keep PROG_VERSION ("vX.Y.Z") and tools/PROG_VERSION.json in sync.

from __future__ import annotations

import json
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Optional


#— Matches e.g.:
#—   const char* PROG_VERSION = "v1.2.3";
#—   const char* PROG_VERSION = "v1.2.3 <extra text>";
#— We only extract/replace the first semantic version token vX.Y.Z.
PROG_VERSION_LINE_REGEX = re.compile(
  r"""
  (?P<prefix>.*?\bPROG_VERSION\b.*?=\s*"?[^;\n]*?)
  v(?P<major>\d+)\.(?P<minor>\d+)\.(?P<patch>\d+)
  (?P<suffix>[^;\n]*"?\s*;.*)
  """,
  re.VERBOSE,
)

#— Fallback: allow formats where '=' and quotes differ, but still contain "vX.Y.Z"
PROG_VERSION_ANYWHERE_REGEX = re.compile(r'\bPROG_VERSION\b[^\n;]*?v(\d+)\.(\d+)\.(\d+)')

#— Fallback replacement: replace the first semantic version token after PROG_VERSION.
PROG_VERSION_ANYWHERE_REPLACE_REGEX = re.compile(
  r'(?P<prefix>\bPROG_VERSION\b[^\n;]*?)v(?P<major>\d+)\.(?P<minor>\d+)\.(?P<patch>\d+)'
)


@dataclass
class SemVer:
  major: int
  minor: int
  patch: int

  def __str__(self) -> str:
    return f"v{self.major}.{self.minor}.{self.patch}"


def run(cmd: list[str]) -> tuple[int, str]:
  proc = subprocess.run(cmd, text=True, capture_output=True)
  out = (proc.stdout or "") + (proc.stderr or "")
  return proc.returncode, out.strip()


def repo_root() -> Path:
  rc, out = run(["git", "rev-parse", "--show-toplevel"])
  if rc != 0 or not out:
    raise RuntimeError("Not a git repository.")
  return Path(out)


def staged_source_files(repo: Path) -> list[Path]:
  #— Only staged (added/copied/modified) C/C++ header/source files.
  rc, out = run(["git", "diff", "--cached", "--name-only", "--diff-filter=ACM"])
  if rc != 0:
    return []

  paths: list[Path] = []
  for line in out.splitlines():
    p = (repo / line.strip())
    if p.suffix.lower() in [".c", ".cpp", ".h"] and p.exists():
      paths.append(p)
  return paths


def tracked_source_files(repo: Path) -> list[Path]:
  #— Tracked C/C++ header/source files (repo-wide), not just staged.
  rc, out = run(["git", "ls-files"])
  if rc != 0:
    return []

  paths: list[Path] = []
  for line in out.splitlines():
    rel = line.strip()
    if not rel:
      continue
    p = repo / rel
    if p.suffix.lower() in [".c", ".cpp", ".h"] and p.exists():
      paths.append(p)
  return paths


def working_tree_source_files(repo: Path) -> list[Path]:
  #— Fallback scan for source files in working tree (also finds untracked files).
  paths: list[Path] = []
  for p in repo.rglob("*"):
    if not p.is_file():
      continue
    if ".git" in p.parts:
      continue
    if p.suffix.lower() in [".c", ".cpp", ".h"]:
      paths.append(p)
  return paths


def is_match_in_comment(text: str, match_start: int) -> bool:
  #— Ignore matches that appear inside C/C++ comments.
  before = text[:match_start]
  if before.rfind("/*") > before.rfind("*/"):
    return True

  line_start = text.rfind("\n", 0, match_start) + 1
  line_prefix = text[line_start:match_start]
  if line_prefix.lstrip().startswith("//"):
    return True

  return False


def find_first_uncommented_match(pattern: re.Pattern, text: str) -> Optional[re.Match]:
  pos = 0
  while True:
    m = pattern.search(text, pos)
    if not m:
      return None
    if not is_match_in_comment(text, m.start()):
      return m
    pos = m.end()


def read_prog_version(path: Path) -> Optional[SemVer]:
  text = path.read_text(encoding="utf-8", errors="replace")

  #— Fast fail if token not present.
  if "PROG_VERSION" not in text:
    return None

  m = find_first_uncommented_match(PROG_VERSION_LINE_REGEX, text)
  if m:
    return SemVer(int(m.group("major")), int(m.group("minor")), int(m.group("patch")))

  m2 = find_first_uncommented_match(PROG_VERSION_ANYWHERE_REGEX, text)
  if m2:
    return SemVer(int(m2.group(1)), int(m2.group(2)), int(m2.group(3)))

  return None


def write_prog_version(path: Path, new_ver: SemVer) -> bool:
  text = path.read_text(encoding="utf-8", errors="replace")

  def repl(match: re.Match) -> str:
    return f'{match.group("prefix")}{new_ver}{match.group("suffix")}'

  m = find_first_uncommented_match(PROG_VERSION_LINE_REGEX, text)
  if m:
    new_text = text[:m.start()] + repl(m) + text[m.end():]
  else:
    #— Try weaker replacement (first "vX.Y.Z" near PROG_VERSION).
    def repl_anywhere(match: re.Match) -> str:
      return f'{match.group("prefix")}{new_ver}'

    m2 = find_first_uncommented_match(PROG_VERSION_ANYWHERE_REPLACE_REGEX, text)
    if not m2:
      return False
    new_text = text[:m2.start()] + repl_anywhere(m2) + text[m2.end():]

  if new_text != text:
    path.write_text(new_text, encoding="utf-8")
  return True


def load_json(path: Path) -> tuple[SemVer, Optional[str]]:
  data = json.loads(path.read_text(encoding="utf-8"))
  ver = SemVer(int(data["major"]), int(data["minor"]), int(data["patch"]))
  version_file = data.get("versionFile")
  if version_file is not None:
    version_file = str(version_file)
  return ver, version_file


def save_json(path: Path, ver: SemVer, version_file: Optional[str]) -> None:
  path.parent.mkdir(parents=True, exist_ok=True)
  data: dict[str, object] = {"major": ver.major, "minor": ver.minor, "patch": ver.patch}
  if version_file:
    data["versionFile"] = version_file
  path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def compute_updated(new_in_code: SemVer, old_in_json: SemVer) -> SemVer:
  if new_in_code.major > old_in_json.major:
    return SemVer(new_in_code.major, 0, 0)

  if new_in_code.minor > old_in_json.minor:
    return SemVer(new_in_code.major, new_in_code.minor, 0)

  return SemVer(new_in_code.major, new_in_code.minor, old_in_json.patch + 1)


def git_add(repo: Path, paths: list[Path]) -> None:
  rels = [str(p.relative_to(repo)) for p in paths]
  run(["git", "add", "--"] + rels)


def find_version_file(repo: Path) -> tuple[Path, SemVer]:
  #— Scan the repo for the file containing PROG_VERSION.
  candidates: list[tuple[Path, SemVer]] = []

  files_to_scan: list[Path] = []
  seen: set[Path] = set()

  for f in tracked_source_files(repo):
    if f not in seen:
      files_to_scan.append(f)
      seen.add(f)

  for f in working_tree_source_files(repo):
    if f not in seen:
      files_to_scan.append(f)
      seen.add(f)

  for f in sorted(files_to_scan):
    ver = read_prog_version(f)
    if ver is not None:
      candidates.append((f, ver))

  if not candidates:
    raise RuntimeError("Could not find PROG_VERSION in any tracked .c/.cpp/.h file.")

  if len(candidates) > 1:
    files = "\n".join([str(p.relative_to(repo)) for (p, _) in candidates])
    raise RuntimeError(
      "Found PROG_VERSION in multiple files. Make it unique or pin it via tools/PROG_VERSION.json versionFile.\n"
      + files
    )

  return candidates[0]


def main() -> int:
  repo = repo_root()
  json_path = repo / "tools" / "PROG_VERSION.json"

  #— Only bump when a relevant C/C++ file is staged.
  staged = staged_source_files(repo)
  if not staged:
    return 0

  version_file: Optional[Path] = None
  new_in_code: Optional[SemVer] = None
  pinned_version_file: Optional[str] = None

  if json_path.exists():
    old_json, pinned_version_file = load_json(json_path)
    if pinned_version_file:
      pinned_path = repo / pinned_version_file
      if not pinned_path.exists():
        print(
          f"[bumpProgVersion] ERROR: versionFile in tools/PROG_VERSION.json does not exist: {pinned_version_file}",
          file=sys.stderr,
        )
        return 1

      ver = read_prog_version(pinned_path)
      if ver is None:
        print(
          f"[bumpProgVersion] ERROR: versionFile does not contain a readable PROG_VERSION: {pinned_version_file}",
          file=sys.stderr,
        )
        return 1

      version_file = pinned_path
      new_in_code = ver

  if version_file is None or new_in_code is None:
    try:
      version_file, new_in_code = find_version_file(repo)
      pinned_version_file = str(version_file.relative_to(repo))
    except Exception as e:
      print(f"[bumpProgVersion] ERROR: {e}", file=sys.stderr)
      return 1

  #— If json doesn't exist: create it from current PROG_VERSION and stop.
  if not json_path.exists():
    save_json(json_path, new_in_code, pinned_version_file)
    git_add(repo, [json_path])
    print(
      f"[bumpProgVersion] Created tools/PROG_VERSION.json from PROG_VERSION={new_in_code} "
      f"(versionFile={pinned_version_file})."
    )
    return 0

  old_json, _ = load_json(json_path)
  updated = compute_updated(new_in_code, old_json)

  changed: list[Path] = []

  #— Always keep json in sync (and persist versionFile).
  save_json(json_path, updated, pinned_version_file)
  changed.append(json_path)

  #— Update code if needed.
  if str(updated) != str(new_in_code):
    if not write_prog_version(version_file, updated):
      print(
        f"[bumpProgVersion] ERROR: Could not update PROG_VERSION in {version_file.relative_to(repo)}",
        file=sys.stderr,
      )
      return 1
    changed.append(version_file)

  if changed:
    git_add(repo, changed)
    print(
      f"[bumpProgVersion] PROG_VERSION -> {updated} "
      f"(file={version_file.relative_to(repo)}, synced with tools/PROG_VERSION.json)."
    )

  return 0


if __name__ == "__main__":
  raise SystemExit(main())
