#!/usr/bin/env python3
"""Convert FluidSynth Doxygen .txt usage docs to Markdown for Zensical/MkDocs.

Usage:
    python3 txt2md.py <input.txt> <output.md>

Or batch convert:
    python3 txt2md.py --batch <usage_dir> <output_dir>
"""

import re
import sys
from pathlib import Path

# Map from Doxygen \page IDs to (relative_md_path, title) for cross-references
PAGE_MAP = {
    "CreatingSettings":   ("creating-settings.md",  "Creating and changing the settings"),
    "CreatingSynth":      ("creating-synth.md",      "Creating the synthesizer"),
    "LoadingSoundfonts":  ("loading-soundfonts.md",  "Loading and managing SoundFonts"),
    "CreatingAudioDriver":("audio-driver.md",        "Creating the audio driver"),
    "UsingSynth":         ("using-synth.md",         "Using the synthesizer without an audio driver"),
    "SendingMIDI":        ("sending-midi.md",        "Sending MIDI events"),
    "RealtimeMIDI":       ("realtime-midi.md",       "Creating a real-time MIDI driver"),
    "MIDIPlayer":         ("midi-player.md",         "Loading and playing a MIDI file"),
    "FileRenderer":       ("file-renderer.md",       "Fast file renderer for non-realtime MIDI file rendering"),
    "MIDIPlayerMem":      ("midi-player-mem.md",     "Playing a MIDI file from memory"),
    "MIDIRouter":         ("midi-router.md",         "Real-time MIDI router"),
    "Sequencer":          ("sequencer.md",           "Using the MIDI sequencer"),
    "Shell":              ("shell.md",               "Shell interface"),
    "Multi-channel":      ("multi-channel.md",       "Multi-channel audio rendering"),
    "synth-context":      ("synth-context.md",       'Understanding the "synthesis context"'),
    "Advanced":           ("advanced.md",            "Advanced features"),
    "UsageGuide":         ("index.md",               "Usage Guide"),
    "RecentChanges":      ("../changelog.md",        "Recent Changes"),
    "ReverbOverview":     ("../reverbators.md",      "Reverb Overview"),
    "fluidsettings":      ("../settings/index.md",   "Settings Reference"),
    "deprecated":         ("../deprecated.md",       "Deprecated Functions"),
}

# Map setting group prefixes used in \setting{} to their settings page
SETTING_GROUP_MAP = {
    "synth":        "synth",
    "audio":        "audio",
    "midi":         "midi",
    "player":       "player",
    "shell":        "shell",
    "reverb":       "synth",  # synth.reverb.*
    "chorus":       "synth",  # synth.chorus.*
}


def setting_to_link(setting_id: str) -> str:
    """Convert a \\setting{X} reference to a Markdown link.

    The argument may use either underscore (e.g. "synth_polyphony") or dot
    (e.g. "synth.sample-rate") as the group/name separator.  The anchor
    produced by the XSL stylesheet always uses the form
    "settings_<group>_<name>", so we normalise the separator to underscore.
    """
    # Normalise: split off the group at the first dot or underscore
    first_dot = setting_id.find(".")
    first_us  = setting_id.find("_")

    if first_dot == -1 and first_us == -1:
        # No separator – can't determine group, just render as inline code
        return f"`{setting_id}`"

    if first_dot == -1:
        sep_pos, sep = first_us, "_"
    elif first_us == -1:
        sep_pos, sep = first_dot, "."
    else:
        sep_pos = min(first_dot, first_us)
        sep = setting_id[sep_pos]

    group = setting_id[:sep_pos]
    name  = setting_id[sep_pos + 1:]

    # Display name always uses dots
    display = f"{group}.{name}"
    # Anchor uses underscores throughout (matches XSL translate(name, '.', '_'))
    anchor = f"settings_{group}_{name.replace('.', '_')}"
    return f"[`{display}`](../settings/{group}.md#{anchor})"


def convert_doxygen_to_markdown(content: str) -> str:
    """Convert a Doxygen .txt file content to Markdown."""
    # Strip the outer C comment delimiters  /* ... */
    content = re.sub(r'^/\*!\s*', '', content.strip())
    content = re.sub(r'\s*\*/$', '', content)
    content = content.strip()

    # Extract and remove the \page command
    page_match = re.match(r'\\page\s+(\S+)\s+(.*?)$', content, re.MULTILINE)
    if page_match:
        page_title = page_match.group(2).strip()
        content = content[page_match.end():].strip()
        content = f"# {page_title}\n\n{content}"

    # \section ID Title  →  ## Title
    def replace_section(m: re.Match) -> str:
        return f"## {m.group(1).strip()}"
    content = re.sub(r'\\section\s+\S+\s+(.*?)$', replace_section,
                     content, flags=re.MULTILINE)

    # \subsection ID Title  →  ### Title
    def replace_subsection(m: re.Match) -> str:
        return f"### {m.group(1).strip()}"
    content = re.sub(r'\\subsection\s+\S+\s+(.*?)$', replace_subsection,
                     content, flags=re.MULTILINE)

    # \code ... \endcode  →  ```c ... ```
    def replace_code_block(m: re.Match) -> str:
        code = m.group(1).rstrip('\n')
        return f"```c\n{code}\n```"
    content = re.sub(r'\\code\s*\n(.*?)\\endcode',
                     replace_code_block, content, flags=re.DOTALL)

    # \htmlonly ... \endhtmlonly  →  strip (HTML-only content like audio players)
    content = re.sub(r'\\htmlonly\s*\n.*?\\endhtmlonly\s*\n?', '',
                     content, flags=re.DOTALL)

    # \image html <file> "Caption"  →  ![Caption](file)
    content = re.sub(r'\\image\s+html\s+(\S+)\s+"([^"]*)"', r'![\2](\1)', content)
    content = re.sub(r'\\image\s+html\s+(\S+)', r'![image](\1)', content)

    # \setting{X}  →  link to settings documentation
    content = re.sub(r'\\setting\{([^}]+)\}', lambda m: setting_to_link(m.group(1)), content)

    # \subpage Name  →  [Title](link)
    def replace_subpage(m: re.Match) -> str:
        page_ref = m.group(1)
        if page_ref in PAGE_MAP:
            file_ref, title = PAGE_MAP[page_ref]
            return f"[{title}]({file_ref})"
        return f"`{page_ref}`"
    content = re.sub(r'\\subpage\s+(\S+)', replace_subpage, content)

    # \ref Name  →  link or backtick
    def replace_ref(m: re.Match) -> str:
        ref = m.group(1)
        if ref in PAGE_MAP:
            file_ref, title = PAGE_MAP[ref]
            return f"[{title}]({file_ref})"
        if ref.endswith('.c') or ref.endswith('.h'):
            return f"`{ref}`"
        # API symbol: link to API reference
        return f"[`{ref}`](../api/index.md)"
    content = re.sub(r'\\ref\s+(\S+)', replace_ref, content)

    # \c Word  →  `Word`
    content = re.sub(r'\\c\s+(\S+)', r'`\1`', content)

    # @c Word  →  `Word`
    content = re.sub(r'@c\s+(\S+)', r'`\1`', content)

    # #SYMBOL_NAME  →  [`SYMBOL_NAME`](../api/index.md) (Doxygen auto-link)
    content = re.sub(r'(?<!\w)#([A-Z][A-Z0-9_]+)',
                     r'[`\1`](../api/index.md)', content)

    # HTML entity/tag conversions
    content = re.sub(r'<code>(.*?)</code>', r'`\1`', content, flags=re.DOTALL)
    content = re.sub(r'<strong>(.*?)</strong>', r'**\1**', content, flags=re.DOTALL)
    content = re.sub(r'<em>(.*?)</em>', r'*\1*', content, flags=re.DOTALL)
    content = re.sub(r'<a\s+href="([^"]+)"[^>]*>(.*?)</a>', r'[\2](\1)',
                     content, flags=re.DOTALL)
    content = re.sub(r'<br\s*/?>', '  \n', content)
    content = re.sub(r'&copy;', '©', content)
    content = re.sub(r'&lt;', '<', content)
    content = re.sub(r'&gt;', '>', content)
    content = re.sub(r'&amp;', '&', content)

    # Clean up more than 2 consecutive blank lines
    content = re.sub(r'\n{3,}', '\n\n', content)

    # Escape prose [ ] brackets that aren't Markdown links, to prevent
    # Zensical / MkDocs from treating them as unresolved reference labels.
    content = re.sub(r'(?<![!`])\[([^\]]*)\](?![\(\[#])', r'\\[\1\\]', content)

    return content.strip() + '\n'


def main() -> None:
    args = sys.argv[1:]
    if len(args) >= 2 and args[0] == '--batch':
        input_dir = Path(args[1])
        output_dir = Path(args[2])
        output_dir.mkdir(parents=True, exist_ok=True)

        # File name map: source .txt  →  output .md
        file_map = {
            "_overview.txt":        "index.md",
            "advanced.txt":         "advanced.md",
            "audio_driver.txt":     "audio-driver.md",
            "creating_settings.txt":"creating-settings.md",
            "creating_synth.txt":   "creating-synth.md",
            "file_renderer.txt":    "file-renderer.md",
            "loading_soundfonts.txt":"loading-soundfonts.md",
            "manual_rendering.txt": "using-synth.md",
            "midi_player.txt":      "midi-player.md",
            "midi_player_mem.txt":  "midi-player-mem.md",
            "midi_router.txt":      "midi-router.md",
            "multi_channel.txt":    "multi-channel.md",
            "realtime_midi.txt":    "realtime-midi.md",
            "sending_midi.txt":     "sending-midi.md",
            "sequencer.txt":        "sequencer.md",
            "shell.txt":            "shell.md",
            "synth_context.txt":    "synth-context.md",
        }

        for src_name, dst_name in file_map.items():
            src = input_dir / src_name
            dst = output_dir / dst_name
            if src.exists():
                converted = convert_doxygen_to_markdown(src.read_text(encoding='utf-8'))
                dst.write_text(converted, encoding='utf-8')
                print(f"  {src_name} → {dst_name}")
            else:
                print(f"  WARNING: {src} not found", file=sys.stderr)

    elif len(args) == 2:
        src = Path(args[0])
        dst = Path(args[1])
        converted = convert_doxygen_to_markdown(src.read_text(encoding='utf-8'))
        dst.write_text(converted, encoding='utf-8')
        print(f"{src} → {dst}")
    else:
        print(f"Usage: {sys.argv[0]} <input.txt> <output.md>")
        print(f"       {sys.argv[0]} --batch <usage_dir> <output_dir>")
        sys.exit(1)


if __name__ == '__main__':
    main()
