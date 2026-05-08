#!/usr/bin/env python3
"""Convert Doxygen XML output to Markdown files for the Zensical/MkDocs site.

Usage:
    python3 doxy2md.py <doxygen_xml_dir> <output_dir>

The script parses the Doxygen XML output (produced with GENERATE_XML=YES) and
writes one Markdown file per API group plus an index page.  All API comments
stay in the C source files – only the generated XML is consumed here.
"""

import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Optional


# ---------------------------------------------------------------------------
# Global registry: doxygen refid  →  "filename.md#anchor" (set in main())
# ---------------------------------------------------------------------------
_REFID_LINK_MAP: dict[str, str] = {}
# (legacy alias kept for callers that used the old dict name)
_REFID_FILE_MAP: dict[str, str] = _REFID_LINK_MAP


# ---------------------------------------------------------------------------
# Helpers to render Doxygen XML description nodes to Markdown text
# ---------------------------------------------------------------------------

def _node_text(node: Optional[ET.Element]) -> str:
    """Return the text content (node.text) or empty string."""
    return (node.text or "") if node is not None else ""


def _node_tail(node: Optional[ET.Element]) -> str:
    """Return the tail text (node.tail) or empty string."""
    return (node.tail or "") if node is not None else ""


def desc_to_md(node: Optional[ET.Element], indent: int = 0) -> str:
    """Recursively convert a Doxygen description XML element to Markdown."""
    if node is None:
        return ""

    tag = node.tag
    buf: list[str] = []

    # ---- inline elements ----
    if tag == "ref":
        name = (node.text or "").strip()
        refid = node.get("refid", "")
        # Look up the resolved file+anchor for this refid
        link_target = _REFID_LINK_MAP.get(refid)
        if link_target:
            buf.append(f"[`{name}`]({link_target})")
        else:
            # Fallback: just render as code
            buf.append(f"`{name}`")
        buf.append(_node_tail(node))
        return "".join(buf)

    if tag in ("computeroutput", "literal", "code"):
        buf.append(f"`{node.text or ''}`")
        buf.append(_node_tail(node))
        return "".join(buf)

    if tag == "emphasis":
        buf.append(f"*{node.text or ''}*")
        buf.append(_node_tail(node))
        return "".join(buf)

    if tag == "bold":
        buf.append(f"**{node.text or ''}**")
        buf.append(_node_tail(node))
        return "".join(buf)

    if tag == "ulink":
        url = node.get("url", "")
        text = node.text or url
        buf.append(f"[{text}]({url})")
        buf.append(_node_tail(node))
        return "".join(buf)

    if tag == "sp":
        return " " + _node_tail(node)

    if tag == "linebreak":
        return "  \n" + _node_tail(node)

    if tag == "ndash":
        return "–" + _node_tail(node)

    if tag == "mdash":
        return "—" + _node_tail(node)

    if tag == "nonbreakablespace":
        return "\u00a0" + _node_tail(node)

    # ---- block elements ----

    if tag == "para":
        # A paragraph may contain inline children; assemble them
        # Check whether the paragraph contains structured block elements
        # (parameterlist, simplesect, programlisting) which must not have
        # their internal whitespace collapsed.
        has_blocks = any(
            child.tag in ("parameterlist", "simplesect", "programlisting",
                          "itemizedlist", "orderedlist", "verbatim")
            for child in node
        )
        parts: list[str] = [node.text or ""]
        for child in node:
            parts.append(desc_to_md(child, indent))
        text = "".join(parts).strip()
        if not text:
            return _node_tail(node)
        if not has_blocks:
            # Plain text paragraph: normalise internal whitespace
            text = re.sub(r'\s+', ' ', text)
        return "\n\n" + text + "\n\n" + (_node_tail(node) or "")

    if tag == "programlisting":
        lines: list[str] = []
        codeline_parts: list[str] = []
        for child in node:
            if child.tag == "codeline":
                line_text = "".join(_extract_text_from_codeline(child))
                codeline_parts.append(line_text)
            else:
                codeline_parts.append(child.text or "")
        code = "\n".join(codeline_parts)
        return f"\n\n```c\n{code}\n```\n\n"

    if tag in ("verbatim",):
        return f"\n\n```\n{node.text or ''}\n```\n\n"

    if tag == "itemizedlist":
        items: list[str] = []
        for child in node:
            item_text = desc_to_md(child, indent + 2).strip()
            items.append(f"{'  ' * indent}- {item_text}")
        return "\n\n" + "\n".join(items) + "\n\n"

    if tag == "orderedlist":
        items = []
        for i, child in enumerate(node, 1):
            item_text = desc_to_md(child, indent + 2).strip()
            items.append(f"{'  ' * indent}{i}. {item_text}")
        return "\n\n" + "\n".join(items) + "\n\n"

    if tag == "listitem":
        parts = [node.text or ""]
        for child in node:
            parts.append(desc_to_md(child, indent))
        return " ".join(p.strip() for p in parts if p.strip())

    if tag == "simplesect":
        kind = node.get("kind", "")
        inner = "".join(desc_to_md(c) for c in node).strip()
        kind_labels = {
            "return":  "**Returns:**",
            "see":     "**See also:**",
            "note":    "> **Note:**",
            "warning": "> **Warning:**",
            "attention": "> **Attention:**",
            "since":   "**Since:**",
            "deprecated": "!!! warning \"Deprecated\"\n    ",
        }
        label = kind_labels.get(kind, f"**{kind.capitalize()}:**")
        if kind in ("note", "warning", "attention"):
            return f"\n\n{label} {inner}\n\n"
        if kind == "deprecated":
            return f"\n\n{label}{inner}\n\n"
        return f"\n\n{label} {inner}\n\n"

    if tag == "parameterlist":
        kind = node.get("kind", "param")
        if kind == "param":
            heading = "**Parameters:**\n\n"
        elif kind == "retval":
            heading = "**Return values:**\n\n"
        else:
            heading = f"**{kind.capitalize()}:**\n\n"

        rows: list[str] = ["| Name | Description |", "|------|-------------|"]
        for item in node.findall("parameteritem"):
            names = ", ".join(
                f"`{n.text}`" for n in item.findall("parameternamelist/parametername")
            )
            desc_parts = [desc_to_md(d).strip() for d in item.findall("parameterdescription")]
            desc_text = " ".join(p for p in desc_parts if p)
            rows.append(f"| {names} | {desc_text} |")

        return "\n\n" + heading + "\n".join(rows) + "\n\n"

    if tag in ("detaileddescription", "briefdescription", "inbodydescription"):
        parts = [node.text or ""]
        for child in node:
            parts.append(desc_to_md(child, indent))
        return "".join(parts)

    # Default: recurse into children, preserve text/tail
    parts = [node.text or ""]
    for child in node:
        parts.append(desc_to_md(child, indent))
    parts.append(_node_tail(node))
    return "".join(parts)


def _extract_text_from_codeline(codeline: ET.Element) -> list[str]:
    """Extract plain text from a <codeline> element."""
    parts: list[str] = []
    for highlight in codeline:
        parts.append(highlight.text or "")
        for ref in highlight:
            parts.append(ref.text or "")
            parts.append(ref.tail or "")
        parts.append(highlight.tail or "")
    return parts


def clean_md(text: str) -> str:
    """Normalise whitespace in generated Markdown."""
    # Collapse more than 2 consecutive blank lines
    text = re.sub(r'\n{3,}', '\n\n', text)
    # Escape [ ... ] prose brackets that aren't Markdown links [text](url).
    # This prevents Zensical / MkDocs from treating them as unresolved
    # reference-style link labels.
    text = re.sub(r'(?<![!`])\[([^\]]*)\](?![\(\[#])', r'\[\1\]', text)
    return text.strip()


# ---------------------------------------------------------------------------
# Render a single compound (group) to Markdown
# ---------------------------------------------------------------------------

def render_function(memberdef: ET.Element) -> str:
    """Render a <memberdef kind="function"> to a Markdown section."""
    name = (memberdef.findtext("name") or "").strip()
    ret_type = "".join(memberdef.find("type").itertext()).strip() if memberdef.find("type") is not None else ""
    argsstring = (memberdef.findtext("argsstring") or "").strip()
    member_id = memberdef.get("id", "")

    brief = desc_to_md(memberdef.find("briefdescription")).strip()
    detail = desc_to_md(memberdef.find("detaileddescription")).strip()

    # Build C-style function signature
    # Strip FLUIDSYNTH_API / FLUID_DEPRECATED macros from return type for display
    ret_display = re.sub(r'\b(FLUIDSYNTH_API|FLUID_DEPRECATED)\b\s*', '', ret_type).strip()
    signature = f"{ret_display} {name}{argsstring}"
    # Use the function name as the anchor id (clean, readable, Zensical-compatible)
    anchor = name

    lines: list[str] = []
    lines.append(f"\n### `{name}()` {{#{anchor}}}\n")
    lines.append(f"\n```c\n{signature}\n```\n")

    if brief:
        lines.append(f"\n{brief}\n")
    if detail:
        lines.append(f"\n{detail}\n")

    return "".join(lines)


def render_typedef(memberdef: ET.Element) -> str:
    """Render a <memberdef kind="typedef"> to a Markdown section."""
    name = (memberdef.findtext("name") or "").strip()
    definition = "".join(memberdef.find("definition").itertext()).strip() \
        if memberdef.find("definition") is not None else name
    member_id = memberdef.get("id", "")

    brief = desc_to_md(memberdef.find("briefdescription")).strip()
    detail = desc_to_md(memberdef.find("detaileddescription")).strip()

    lines: list[str] = []
    lines.append(f"\n### `{name}` {{#{name}}}\n")
    lines.append(f"\n```c\ntypedef {definition};\n```\n")
    if brief:
        lines.append(f"\n{brief}\n")
    if detail:
        lines.append(f"\n{detail}\n")
    return "".join(lines)


def render_enum(memberdef: ET.Element) -> str:
    """Render a <memberdef kind="enum"> to a Markdown section."""
    name = (memberdef.findtext("name") or "").strip()
    member_id = memberdef.get("id", "")

    brief = desc_to_md(memberdef.find("briefdescription")).strip()
    detail = desc_to_md(memberdef.find("detaileddescription")).strip()

    lines: list[str] = []
    lines.append(f"\n### `{name}` {{#{name}}}\n")
    if brief:
        lines.append(f"\n{brief}\n")

    # Enum values
    enum_values = memberdef.findall("enumvalue")
    if enum_values:
        rows = ["| Value | Description |", "|-------|-------------|"]
        for ev in enum_values:
            val_name = ev.findtext("name") or ""
            val_brief = desc_to_md(ev.find("briefdescription")).strip()
            rows.append(f"| `{val_name}` | {val_brief} |")
        lines.append("\n" + "\n".join(rows) + "\n")

    if detail:
        lines.append(f"\n{detail}\n")
    return "".join(lines)


def render_define(memberdef: ET.Element) -> str:
    """Render a <memberdef kind="define"> to a Markdown section."""
    name = (memberdef.findtext("name") or "").strip()
    member_id = memberdef.get("id", "")

    brief = desc_to_md(memberdef.find("briefdescription")).strip()
    detail = desc_to_md(memberdef.find("detaileddescription")).strip()

    lines: list[str] = []
    lines.append(f"\n### `{name}` {{#{name}}}\n")
    if brief:
        lines.append(f"\n{brief}\n")
    if detail:
        lines.append(f"\n{detail}\n")
    return "".join(lines)


# Kind → section heading label
SECTION_KIND_LABELS = {
    "func":     "Functions",
    "typedef":  "Types",
    "enum":     "Enumerations",
    "define":   "Macros",
    "var":      "Variables",
}

MEMBER_RENDERERS = {
    "function": render_function,
    "typedef":  render_typedef,
    "enum":     render_enum,
    "define":   render_define,
}


def render_group(compound_xml: Path) -> Optional[tuple[str, str, str]]:
    """Parse a Doxygen group XML file and return (group_id, title, markdown_text)."""
    tree = ET.parse(compound_xml)
    root = tree.getroot()
    compounddef = root.find("compounddef")
    if compounddef is None:
        return None

    group_id  = compounddef.get("id", "")
    title     = (compounddef.findtext("title") or group_id).strip()

    brief  = desc_to_md(compounddef.find("briefdescription")).strip()
    detail = desc_to_md(compounddef.find("detaileddescription")).strip()

    lines: list[str] = [f"# {title}\n"]

    if brief:
        lines.append(f"\n{brief}\n")
    if detail:
        lines.append(f"\n{detail}\n")

    # Nested groups (subgroups)
    inner_groups = compounddef.findall("innergroup")
    if inner_groups:
        lines.append("\n## Subgroups\n")
        for ig in inner_groups:
            sub_id    = ig.get("refid", "")
            sub_title = (ig.text or sub_id).strip()
            # Convert group__sub_id to a filename (same convention used for output files)
            sub_file  = sub_id.replace("group__", "").replace("__", "-") + ".md"
            lines.append(f"\n- [{sub_title}]({sub_file})\n")

    # Members organised by section kind
    for sectiondef in compounddef.findall("sectiondef"):
        kind = sectiondef.get("kind", "")
        heading = SECTION_KIND_LABELS.get(kind, kind.capitalize())
        members = sectiondef.findall("memberdef")
        if not members:
            continue
        lines.append(f"\n## {heading}\n")
        for memberdef in members:
            member_kind = memberdef.get("kind", "")
            renderer = MEMBER_RENDERERS.get(member_kind)
            if renderer:
                lines.append(renderer(memberdef))

    return group_id, title, clean_md("".join(lines))


# ---------------------------------------------------------------------------
# Main conversion logic
# ---------------------------------------------------------------------------

def group_id_to_filename(group_id: str) -> str:
    """Convert a Doxygen group refid to a Markdown file name.

    E.g. "group__midi__messages" → "midi-messages.md"
    """
    name = group_id
    name = re.sub(r'^group__', '', name)
    name = name.replace('__', '-')
    return name + ".md"


def main() -> None:
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <doxygen_xml_dir> <output_dir>")
        sys.exit(1)

    xml_dir    = Path(sys.argv[1])
    output_dir = Path(sys.argv[2])
    output_dir.mkdir(parents=True, exist_ok=True)

    index_xml = xml_dir / "index.xml"
    if not index_xml.exists():
        print(f"ERROR: {index_xml} not found.  Run doxygen with GENERATE_XML=YES first.",
              file=sys.stderr)
        sys.exit(1)

    # Parse the top-level index to find all groups
    index_tree = ET.parse(index_xml)
    index_root = index_tree.getroot()

    groups: dict[str, tuple[str, str]] = {}  # refid → (title, filename)

    # ----- First pass: build the refid → "file.md#name" link map -----
    # This allows cross-references (e.g. <ref refid="group__X_1gY">) to link
    # to the correct output file using the human-readable member name as anchor.
    for compound in index_root.findall("compound[@kind='group']"):
        refid = compound.get("refid", "")
        compound_xml = xml_dir / f"{refid}.xml"
        if not compound_xml.exists():
            continue
        filename = group_id_to_filename(refid)
        try:
            tree = ET.parse(compound_xml)
        except ET.ParseError:
            continue
        # Map the compound (group) refid to its index file
        _REFID_LINK_MAP[refid] = filename
        for memberdef in tree.getroot().iter("memberdef"):
            member_id   = memberdef.get("id", "")
            member_name = (memberdef.findtext("name") or "").strip()
            if member_id and member_name:
                _REFID_LINK_MAP[member_id] = f"{filename}#{member_name}"

    # ----- Second pass: render and write the Markdown files -----
    for compound in index_root.findall("compound[@kind='group']"):
        refid = compound.get("refid", "")
        title = compound.findtext("name") or refid
        compound_xml = xml_dir / f"{refid}.xml"
        if not compound_xml.exists():
            print(f"  WARNING: {compound_xml} not found, skipping", file=sys.stderr)
            continue

        result = render_group(compound_xml)
        if result is None:
            continue

        group_id, group_title, md_text = result
        filename = group_id_to_filename(group_id)
        out_path = output_dir / filename
        out_path.write_text(md_text + "\n", encoding="utf-8")
        groups[refid] = (group_title, filename)
        print(f"  {refid} → {filename}")

    # Write api/index.md – overview listing all top-level groups
    # A "top-level" group is one that is not listed as an innergroup in any other group
    nested_ids: set[str] = set()
    for refid in groups:
        compound_xml = xml_dir / f"{refid}.xml"
        if compound_xml.exists():
            tree = ET.parse(compound_xml)
            for ig in tree.getroot().iter("innergroup"):
                nested_ids.add(ig.get("refid", ""))

    top_level = [(refid, title, fname)
                 for refid, (title, fname) in sorted(groups.items(), key=lambda x: x[1][0])
                 if refid not in nested_ids]

    index_lines = [
        "# API Reference\n\n",
        "This is the complete FluidSynth public API reference, grouped by functionality.\n\n",
    ]
    for refid, title, fname in top_level:
        index_lines.append(f"- [{title}]({fname})\n")

    (output_dir / "index.md").write_text("".join(index_lines), encoding="utf-8")
    print(f"  index.md written ({len(top_level)} top-level groups)")


if __name__ == "__main__":
    main()
