#!/usr/bin/env python3
"""Generate a runtime zensical.toml with all API pages listed in the nav.

The static zensical.toml keeps a placeholder section marked with
  # __API_NAV_BEGIN__
  ...
  # __API_NAV_END__
This script scans doc/wiki/api/ for generated .md files, builds the full nav
list, and replaces the placeholder section.  The result is written to the path
given as the first argument (typically a temp file in the build directory).

Usage:
    python3 gen-zensical-config.py <output_config.toml> <docs_dir> [<src_zensical.toml>]

If <src_zensical.toml> is omitted, the script looks for zensical.toml in the
parent of docs_dir, then two levels up (repo root).
"""

import re
import sys
from pathlib import Path


def natural_sort_key(name: str) -> tuple:
    """Sort key: index.md first, then known files, then alphabetical."""
    order = {
        "index.md": 0,
        "recent-changes.md": 1,
        "deprecated.md": 2,
        "examples.md": 3,
    }
    return (order.get(name, 99), name)


def gather_api_nav(api_dir: Path) -> list:
    """Return sorted nav paths (relative to docs_dir) for the api/ directory."""
    if not api_dir.is_dir():
        return ["api/index.md"]

    entries = []

    # Collect top-level .md files
    for p in sorted(api_dir.glob("*.md"), key=lambda p: natural_sort_key(p.name)):
        entries.append(f"api/{p.name}")

    # Collect per-example pages in api/examples/ subdirectory, if present
    examples_dir = api_dir / "examples"
    if examples_dir.is_dir():
        sub_entries = sorted(
            [f"api/examples/{p.name}" for p in examples_dir.glob("*.md")],
            key=lambda s: (0 if s.endswith("index.md") else 1, s),
        )
        entries.extend(sub_entries)

    return entries if entries else ["api/index.md"]


def build_nav_block(nav_entries: list, indent: str = "            ") -> str:
    """Render the API Reference nav section as TOML lines."""
    lines = [f'{indent}# __API_NAV_BEGIN__']
    lines.append(f'{indent}{{"🔍 API Reference" = [')
    for entry in nav_entries:
        lines.append(f'{indent}    "{entry}",')
    lines.append(f'{indent}]}},')
    lines.append(f'{indent}# __API_NAV_END__')
    return "\n".join(lines)


def patch_config(src_text: str, nav_entries: list) -> str:
    """Replace the __API_NAV_BEGIN__ ... __API_NAV_END__ block."""
    pattern = re.compile(
        r'[ \t]*# __API_NAV_BEGIN__\n.*?[ \t]*# __API_NAV_END__',
        re.DOTALL,
    )

    m = pattern.search(src_text)
    if not m:
        print("WARNING: __API_NAV_BEGIN__ marker not found in zensical.toml", file=sys.stderr)
        return src_text

    # Detect indentation from the BEGIN marker
    first_line = m.group(0).split("\n")[0]
    indent = first_line[: len(first_line) - len(first_line.lstrip())]

    replacement = build_nav_block(nav_entries, indent=indent)
    return pattern.sub(replacement, src_text)


def find_config_source(docs_dir: Path) -> Path:
    """Locate zensical.toml: look next to docs_dir, then two levels up."""
    for candidate in [docs_dir.parent / "zensical.toml",
                      docs_dir.parent.parent / "zensical.toml"]:
        if candidate.exists():
            return candidate
    return None


def main() -> None:
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <output_config.toml> <docs_dir> [<src_zensical.toml>]",
              file=sys.stderr)
        sys.exit(1)

    output_path = Path(sys.argv[1])
    docs_dir = Path(sys.argv[2])

    if len(sys.argv) >= 4:
        config_src = Path(sys.argv[3])
    else:
        config_src = find_config_source(docs_dir)
        if config_src is None:
            print(f"ERROR: zensical.toml not found near {docs_dir}", file=sys.stderr)
            sys.exit(1)

    src_text = config_src.read_text(encoding="utf-8")

    api_dir = docs_dir / "api"
    nav_entries = gather_api_nav(api_dir)
    print(f"  Found {len(nav_entries)} API nav entries")

    patched = patch_config(src_text, nav_entries)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(patched, encoding="utf-8")
    print(f"  Runtime config written to {output_path}")


if __name__ == "__main__":
    main()
