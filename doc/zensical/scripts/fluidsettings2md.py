#!/usr/bin/env python3
"""Convert FluidSynth fluidsettings.xml to Markdown for the Zensical/MkDocs site.

Usage:
    python3 fluidsettings2md.py <fluidsettings.xml> <output_dir>

The script writes one Markdown file per settings group plus an index.md.  The
anchor IDs match those produced by the legacy XSLT stylesheet so that
\\setting{} cross-references in the usage guide pages remain valid.
"""

import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def clean_html(text: str) -> str:
    """Convert inline HTML in setting descriptions to Markdown."""
    if not text:
        return ""
    text = re.sub(r'<br\s*/?>', '  \n', text)
    text = re.sub(r'<code>(.*?)</code>', r'`\1`', text, flags=re.DOTALL)
    text = re.sub(r'<strong>(.*?)</strong>', r'**\1**', text, flags=re.DOTALL)
    text = re.sub(r'<em>(.*?)</em>', r'*\1*', text, flags=re.DOTALL)
    text = re.sub(r'<a\s+href="([^"]+)"[^>]*>(.*?)</a>', r'[\2](\1)',
                  text, flags=re.DOTALL)
    text = re.sub(r'<[^>]+>', '', text)   # strip remaining tags
    text = re.sub(r'&lt;', '<', text)
    text = re.sub(r'&gt;', '>', text)
    text = re.sub(r'&amp;', '&', text)
    text = re.sub(r'&copy;', '©', text)
    # Escape prose [ ] brackets that aren't Markdown links to prevent
    # Zensical from treating them as unresolved reference-style link labels.
    text = re.sub(r'(?<![!`])\[([^\]]*)\](?![\(\[#])', r'\\[\1\\]', text)
    return text.strip()


def setting_anchor(group: str, name: str) -> str:
    """Return the HTML anchor ID for a setting.

    Matches the section ID produced by doc/doxygen/fluidsettings.xsl:
        settings_<group>_<translate(name, '.', '_')>
    """
    name_escaped = name.replace(".", "_")
    return f"settings_{group}_{name_escaped}"


# ---------------------------------------------------------------------------
# Main conversion
# ---------------------------------------------------------------------------

def main() -> None:
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <fluidsettings.xml> <output_dir>")
        sys.exit(1)

    xml_file   = Path(sys.argv[1])
    output_dir = Path(sys.argv[2])
    output_dir.mkdir(parents=True, exist_ok=True)

    tree = ET.parse(xml_file)
    root = tree.getroot()

    # Sort groups by label
    groups = sorted(root, key=lambda e: e.get("label", e.tag))

    # ---- index.md ----
    index_lines: list[str] = [
        "# Settings Reference\n\n",
        "FluidSynth settings are organised into the following groups.\n",
        "Each setting has a name in *group.name* dotted notation, a type,\n",
        "a default value, and an optional range.\n\n",
    ]
    for group_elem in groups:
        group_label = group_elem.get("label", group_elem.tag)
        group_tag   = group_elem.tag
        index_lines.append(f"- [{group_label}]({group_tag}.md)\n")

    (output_dir / "index.md").write_text("".join(index_lines), encoding="utf-8")
    print("  index.md")

    # ---- per-group pages ----
    for group_elem in groups:
        group_tag   = group_elem.tag
        group_label = group_elem.get("label", group_tag)

        lines: list[str] = [f"# {group_label}\n\n"]

        for setting_elem in group_elem.findall("setting"):
            name        = setting_elem.findtext("name") or ""
            type_       = setting_elem.findtext("type") or ""
            default_val = setting_elem.findtext("def") or ""
            min_val     = setting_elem.findtext("min") or ""
            max_val     = setting_elem.findtext("max") or ""
            desc_raw    = setting_elem.findtext("desc") or ""
            deprecated  = setting_elem.findtext("deprecated") or ""
            realtime_el = setting_elem.find("realtime")
            realtime    = realtime_el is not None
            realtime_note = (realtime_el.text or "").strip() if realtime else ""

            full_name = f"{group_tag}.{name}"
            anchor    = setting_anchor(group_tag, name)

            lines.append(f"\n## `{full_name}` {{#{anchor}}}\n\n")

            if deprecated:
                dep_text = clean_html(deprecated)
                lines.append(f"!!! warning \"Deprecated\"\n    {dep_text}\n\n")

            if realtime:
                rt_text = clean_html(realtime_note) if realtime_note else \
                    "This setting can be changed at run time."
                lines.append(f"!!! tip \"Real-time\"\n    {rt_text}\n\n")

            # Property table
            lines.append("| Property | Value |\n|----------|-------|\n")
            lines.append(f"| Type | `{type_}` |\n")
            lines.append(f"| Default | `{default_val}` |\n")
            if min_val:
                lines.append(f"| Min | `{min_val}` |\n")
            if max_val:
                lines.append(f"| Max | `{max_val}` |\n")
            lines.append("\n")

            desc = clean_html(desc_raw)
            if desc:
                lines.append(f"{desc}\n")

        filename = f"{group_tag}.md"
        (output_dir / filename).write_text("".join(lines), encoding="utf-8")
        print(f"  {filename}")


if __name__ == "__main__":
    main()
