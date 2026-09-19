#!/usr/bin/env bash
# run-xslt.sh  --  XSLT-based XML-to-Markdown conversion driver
# ==============================================================
# Usage:
#   run-xslt.sh fluidsettings \
#       <fluidsettings.xml> <output_dir> [<fluidsettings2md.xsl>]
#
#   run-xslt.sh doxy \
#       <doxygen_xml_dir> <output_dir> [<doxy2md.xsl>]
#
#   run-xslt.sh pages \
#       <doxygen_xml_dir> <output_dir> [<doxy2md.xsl>] [<api_prefix>]
#       Converts Doxygen \page compound XML to Markdown.
#       <api_prefix>  relative path prefix to prepend to API symbol links in the
#                     refmap (default "../api/"); use "" when output_dir IS the api dir.
#
# Requirements: xsltproc (from libxslt)
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Inline XSL to list group names from a fluidsettings.xml file.
# Outputs one group name per line (e.g. "synth", "audio", "midi" …).
# Used by both doxy and pages modes to build per-group page-level refmap entries.
FLUIDSETTINGS_GROUPS_XSL='<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text"/>
  <xsl:template match="/">
    <xsl:for-each select="/fluidsettings/*">
      <xsl:value-of select="name(.)"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:template>
</xsl:stylesheet>'

usage() {
    echo "Usage: $0 {fluidsettings|doxy|pages|examples} <input> <output_dir> [<xsl_file>]"
    exit 1
}

if [[ "$#" -lt 3 ]]; then usage; fi

MODE="$1"
INPUT="$2"
OUTPUT_DIR="$3"
XSL_DEFAULT_FLUIDSETTINGS="$SCRIPT_DIR/fluidsettings2md.xsl"
XSL_DEFAULT_DOXY="$SCRIPT_DIR/doxy2md.xsl"

# ---------------------------------------------------------------------------
# Helper: split a stream that uses "__FILE__: <name>" sentinels into files
# ---------------------------------------------------------------------------
split_into_files() {
    local out_dir="$1"
    local current_file=""
    while IFS= read -r line; do
        if [[ "$line" =~ ^__FILE__:\ *(.+)$ ]]; then
            current_file="${out_dir}/${BASH_REMATCH[1]}"
            # truncate/create
            : > "$current_file"
        elif [[ -n "$current_file" ]]; then
            printf '%s\n' "$line" >> "$current_file"
        fi
    done
}

# ---------------------------------------------------------------------------
# MODE: fluidsettings
# ---------------------------------------------------------------------------
if [[ "$MODE" = "fluidsettings" ]]; then
    XML_FILE="$INPUT"
    XSL_FILE="${4:-$XSL_DEFAULT_FLUIDSETTINGS}"

    if [[ ! -f "$XML_FILE" ]]; then
        echo "ERROR: fluidsettings.xml not found: $XML_FILE" >&2
        exit 1
    fi
    if [[ ! -f "$XSL_FILE" ]]; then
        echo "ERROR: XSL stylesheet not found: $XSL_FILE" >&2
        exit 1
    fi

    mkdir -p "$OUTPUT_DIR"
    echo "  Running xsltproc for fluidsettings.xml ..."
    xsltproc "$XSL_FILE" "$XML_FILE" | split_into_files "$OUTPUT_DIR"
    echo "  Settings pages written to $OUTPUT_DIR/"
    exit 0
fi

# ---------------------------------------------------------------------------
# MODE: doxy  (Doxygen XML directory → API Markdown pages)
# ---------------------------------------------------------------------------
if [[ "$MODE" = "doxy" ]]; then
    XML_DIR="$INPUT"
    XSL_FILE="${4:-$XSL_DEFAULT_DOXY}"
    INDEX_XML="$XML_DIR/index.xml"

    if [[ ! -f "$INDEX_XML" ]]; then
        echo "ERROR: Doxygen index.xml not found: $INDEX_XML" >&2
        exit 1
    fi
    if [[ ! -f "$XSL_FILE" ]]; then
        echo "ERROR: XSL stylesheet not found: $XSL_FILE" >&2
        exit 1
    fi

    mkdir -p "$OUTPUT_DIR"

    # ------------------------------------------------------------------
    # First pass: collect all group refids and build the refmap
    # Format: "refid<TAB>filename.md#member_name"
    # ------------------------------------------------------------------
    echo "  First pass: building cross-reference map ..."
    REFMAP_FILE="$(mktemp /tmp/doxy_refmap.XXXXXX)"
    trap 'rm -f "$REFMAP_FILE"' EXIT

    # Inline XSL to list group compound refids from index.xml
    LIST_GROUPS_XSL='<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text"/>
  <xsl:template match="/">
    <xsl:for-each select="//compound[@kind=&apos;group&apos;]">
      <xsl:value-of select="@refid"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:template>
</xsl:stylesheet>'

    # Inline XSL to extract "id TAB name" pairs from a group XML.
    # Outputs both memberdef entries AND enumvalue entries (the latter mapped to
    # their parent enum type name so they link to the correct heading anchor).
    LIST_MEMBERS_XSL='<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text"/>
  <xsl:template match="/">
    <xsl:for-each select="//memberdef">
      <xsl:value-of select="@id"/>
      <xsl:text>&#9;</xsl:text>
      <xsl:value-of select="name"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
    <xsl:for-each select="//enumvalue">
      <xsl:value-of select="@id"/>
      <xsl:text>&#9;</xsl:text>
      <xsl:value-of select="parent::memberdef/name"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:template>
</xsl:stylesheet>'

    # Optional: path to fluidsettings.xml so \setting{} cross-refs can be resolved.
    FLUIDSETTINGS_XML="${5:-}"

    # Collect group compound refids
    GROUP_REFIDS=$(echo "$LIST_GROUPS_XSL" | xsltproc - "$INDEX_XML")

    # For each group XML file: add refid→filename and refid→filename#name entries
    while IFS= read -r refid; do
        [[ -z "$refid" ]] && continue
        group_xml="$XML_DIR/${refid}.xml"
        [[ -f "$group_xml" ]] || continue

        # Compute the output filename from the refid
        filename=$(echo "$refid" \
            | sed 's/^group__//' \
            | sed 's/__/-/g')
        filename="${filename}.md"

        # Map the group compound itself
        printf '%s|%s\n' "$refid" "$filename" >> "$REFMAP_FILE"

        # Extract member pairs and append "member_id|filename#name"
        while IFS=$'\t' read -r member_id member_name; do
            [[ -z "$member_id" ]] && continue
            printf '%s|%s#%s\n' "$member_id" "$filename" "$member_name" >> "$REFMAP_FILE"
        done < <(echo "$LIST_MEMBERS_XSL" | xsltproc - "$group_xml")
    done <<< "$GROUP_REFIDS"

    # ------------------------------------------------------------------
    # Add Doxygen \page refids to the refmap so @sa @ref PageId links work.
    # Keys are the Doxygen page compound IDs; values are paths relative to
    # the generated api/ directory (where the API .md files live).
    # ------------------------------------------------------------------
    declare -A PAGE_REFMAP=(
        ["CreatingSettings"]="../usage/creating-settings.md"
        ["CreatingSynth"]="../usage/creating-synth.md"
        ["LoadingSoundfonts"]="../usage/loading-soundfonts.md"
        ["CreatingAudioDriver"]="../usage/audio-driver.md"
        ["UsingSynth"]="../usage/using-synth.md"
        ["SendingMIDI"]="../usage/sending-midi.md"
        ["RealtimeMIDI"]="../usage/realtime-midi.md"
        ["MIDIPlayer"]="../usage/midi-player.md"
        ["FileRenderer"]="../usage/file-renderer.md"
        ["MIDIPlayerMem"]="../usage/midi-player-mem.md"
        ["MIDIRouter"]="../usage/midi-router.md"
        ["Sequencer"]="../usage/sequencer.md"
        ["Shell"]="../usage/shell.md"
        ["Multi-channel"]="../usage/multi-channel.md"
        ["synth-context"]="../usage/synth-context.md"
        ["Advanced"]="../usage/advanced.md"
        ["UsageGuide"]="../usage/index.md"
        ["RecentChanges"]="recent-changes.md"
        ["ReverbOverview"]="../reverbators.md"
        ["fluidsettings"]="../settings/index.md"
        ["deprecated"]="deprecated.md"
    )
    for page_id in "${!PAGE_REFMAP[@]}"; do
        printf '%s|%s\n' "$page_id" "${PAGE_REFMAP[$page_id]}" >> "$REFMAP_FILE"
    done

    # ------------------------------------------------------------------
    # Add individual \setting{} cross-references from fluidsettings.xml.
    # \setting{GROUP_NAME} expands to \ref settings_GROUP_NAME.
    # Doxygen generates a \section settings_GROUP_NAME inside the page settings_GROUP,
    # and the XML refid for a section within a page is: <page_id>_1<section_id>
    # e.g. settings_synth_1settings_synth_limiter_active
    # The Markdown anchor (from fluidsettings2md.xsl) is: settings_GROUP_NAME
    # (dots replaced by underscores, hyphens preserved).
    # We also add page-level entries for \ref settings_GROUP page references.
    # ------------------------------------------------------------------
    if [[ -n "$FLUIDSETTINGS_XML" ]] && [[ -f "$FLUIDSETTINGS_XML" ]]; then
        echo "  Adding settings cross-references from fluidsettings.xml ..."
        while IFS=$'\t' read -r grp raw_name; do
            [[ -z "$grp" ]] && continue
            translated="$(echo "$raw_name" | tr '.' '_')"
            section_id="settings_${grp}_${translated}"
            doxy_refid="settings_${grp}_1${section_id}"
            printf '%s|../settings/%s.md#%s\n' "$doxy_refid" "$grp" "$section_id" >> "$REFMAP_FILE"
        done < <(xsltproc - "$FLUIDSETTINGS_XML" <<'SETTINGS_XSL'
<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text"/>
  <xsl:template match="/">
    <xsl:for-each select="//setting">
      <xsl:value-of select="name(..)"/>
      <xsl:text>&#9;</xsl:text>
      <xsl:value-of select="name"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:template>
</xsl:stylesheet>
SETTINGS_XSL
        )
        # Add page-level entries for \ref settings_GROUP references
        while IFS= read -r grp; do
            [[ -z "$grp" ]] && continue
            printf 'settings_%s|../settings/%s.md\n' "$grp" "$grp" >> "$REFMAP_FILE"
        done < <(echo "$FLUIDSETTINGS_GROUPS_XSL" | xsltproc - "$FLUIDSETTINGS_XML")
    fi

    REFMAP_TEXT="$(cat "$REFMAP_FILE")"

    # Inline XSL to list innergroup refids from a compound XML
    LIST_INNERGROUPS_XSL='<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text"/>
  <xsl:template match="/">
    <xsl:for-each select="//innergroup">
      <xsl:value-of select="@refid"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:template>
</xsl:stylesheet>'

    # Inline XSL to get a compound title
    GET_TITLE_XSL='<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text"/>
  <xsl:template match="/">
    <xsl:value-of select="//compounddef/title"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>
</xsl:stylesheet>'

    # ------------------------------------------------------------------
    # Second pass: transform each group XML to a Markdown file
    # ------------------------------------------------------------------
    echo "  Second pass: converting group XML files ..."
    while IFS= read -r refid; do
        [[ -z "$refid" ]] && continue
        group_xml="$XML_DIR/${refid}.xml"
        [[ -f "$group_xml" ]] || continue

        filename=$(echo "$refid" \
            | sed 's/^group__//' \
            | sed 's/__/-/g')
        out_file="$OUTPUT_DIR/${filename}.md"

        xsltproc \
            --stringparam refmap-text "$REFMAP_TEXT" \
            "$XSL_FILE" "$group_xml" > "$out_file"
        echo "    $refid → ${filename}.md"
    done <<< "$GROUP_REFIDS"

    # ------------------------------------------------------------------
    # Write api/index.md  (top-level groups only)
    # ------------------------------------------------------------------
    echo "  Writing api/index.md ..."
    {
        echo "# API Reference"
        echo ""
        echo "This is the complete FluidSynth public API reference, grouped by functionality."
        echo ""
    } > "$OUTPUT_DIR/index.md"

    # Determine nested refids by scanning all group XML files for <innergroup>
    declare -A nested_set
    while IFS= read -r refid; do
        [[ -z "$refid" ]] && continue
        group_xml="$XML_DIR/${refid}.xml"
        [[ -f "$group_xml" ]] || continue
        while IFS= read -r inner; do
            [[ -z "$inner" ]] && continue
            nested_set["$inner"]=1
        done < <(echo "$LIST_INNERGROUPS_XSL" | xsltproc - "$group_xml")
    done <<< "$GROUP_REFIDS"

    # Get group titles from each XML
    declare -A group_titles
    while IFS= read -r refid; do
        [[ -z "$refid" ]] && continue
        group_xml="$XML_DIR/${refid}.xml"
        [[ -f "$group_xml" ]] || continue
        title=$(echo "$GET_TITLE_XSL" | xsltproc - "$group_xml" | head -1)
        group_titles["$refid"]="$title"
    done <<< "$GROUP_REFIDS"

    # Print top-level groups sorted by title
    while IFS= read -r refid; do
        [[ -z "$refid" ]] && continue
        [[ "${nested_set[$refid]+_}" ]] && continue   # skip nested groups
        title="${group_titles[$refid]}"
        filename=$(echo "$refid" \
            | sed 's/^group__//' \
            | sed 's/__/-/g')
        echo "- [$title](${filename}.md)" >> "$OUTPUT_DIR/index.md"
    done < <(
        for refid in "${!group_titles[@]}"; do
            printf '%s\t%s\n' "${group_titles[$refid]}" "$refid"
        done | sort | cut -f2
    )

    echo "  Writing api/index.md done."

    # ------------------------------------------------------------------
    # Generate api/deprecated.md (list of deprecated members)
    # ------------------------------------------------------------------
    echo "  Generating deprecated.md ..."
    DEPRECATED_XSL='<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text"/>
  <xsl:template match="/">
    <xsl:for-each select="//memberdef[.//simplesect[@kind=&apos;deprecated&apos;] or .//xrefsect[xreftitle=&apos;Deprecated&apos;]]">
      <xsl:value-of select="name"/>
      <xsl:text>&#9;</xsl:text>
      <xsl:value-of select="normalize-space(briefdescription)"/>
      <xsl:text>&#9;</xsl:text>
      <xsl:value-of select="@id"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:template>
</xsl:stylesheet>'

    {
        printf "# Deprecated API\n\n"
        printf "The following API members have been deprecated and may be removed in a future release.\n\n"
        printf "| Name | Description |\n"
        printf "|------|-------------|\n"

        while IFS= read -r refid; do
            [[ -z "$refid" ]] && continue
            group_xml="$XML_DIR/${refid}.xml"
            [[ -f "$group_xml" ]] || continue

            while IFS=$'\t' read -r member_name member_brief _member_id; do
                [[ -z "$member_name" ]] && continue
                # Compute the filename from the refid
                filename=$(echo "$refid" \
                    | sed 's/^group__//' \
                    | sed 's/__/-/g')
                # The anchor matches the {#member_name} heading IDs emitted by doxy2md.xsl
                printf '| [`%s`](%s.md#%s) | %s |\n' \
                    "$member_name" "$filename" "$member_name" "$member_brief"
            done < <(echo "$DEPRECATED_XSL" | xsltproc - "$group_xml")
        done <<< "$GROUP_REFIDS"
    } > "$OUTPUT_DIR/deprecated.md"
    echo "    → deprecated.md"

    echo "  Done. API pages written to $OUTPUT_DIR/"
    exit 0
fi

# ---------------------------------------------------------------------------
# MODE: pages  (Doxygen page XML → narrative Markdown pages)
# Converts Doxygen \page compound XML files to Markdown.
# Unlike the "doxy" mode (which handles \defgroup API reference pages),
# this mode handles \page narrative documentation (usage guides, changelog …).
#
# Usage:
#   run-xslt.sh pages <doxygen_xml_dir> <output_dir> [<xsl_file>] [<api_prefix>] [<page_id_filter>] [<fluidsettings.xml>]
#
# <api_prefix>         relative path from <output_dir> to the generated api/ directory.
#                      Defaults to "../api/"; use "" when output_dir IS the api dir.
# <page_id_filter>     optional: if given, only this single page ID is processed.
#                      Useful for generating just "RecentChanges" into api/.
# <fluidsettings.xml>  optional: path to fluidsettings.xml; when provided, per-setting
#                      \setting{} cross-references are added to the refmap.
# ---------------------------------------------------------------------------
if [[ "$MODE" = "pages" ]]; then
    XML_DIR="$INPUT"
    XSL_FILE="${4:-$XSL_DEFAULT_DOXY}"
    API_PREFIX="${5:-../api/}"
    PAGE_FILTER="${6:-}"      # optional: restrict to a single page ID
    # CMake strips empty-string arguments from COMMAND lists; treat the
    # sentinel "_nofilter_" as equivalent to an empty filter (process all pages).
    [[ "$PAGE_FILTER" = "_nofilter_" ]] && PAGE_FILTER=""
    FLUIDSETTINGS_XML="${7:-}"  # optional: path to fluidsettings.xml
    INDEX_XML="$XML_DIR/index.xml"

    if [[ ! -f "$INDEX_XML" ]]; then
        echo "ERROR: Doxygen index.xml not found: $INDEX_XML" >&2
        exit 1
    fi
    if [[ ! -f "$XSL_FILE" ]]; then
        echo "ERROR: XSL stylesheet not found: $XSL_FILE" >&2
        exit 1
    fi

    mkdir -p "$OUTPUT_DIR"

    # ------------------------------------------------------------------
    # Static mapping: Doxygen page ID → output Markdown filename
    # PAGE_FILE_MAP contains the usage guide pages (the default set).
    # EXTRA_PAGE_FILE_MAP contains pages that are NOT usage guide pages
    # (e.g. RecentChanges → api/recent-changes.md).
    # When PAGE_FILTER is given, only that page ID is processed.
    # Both maps contribute to the refmap for cross-reference resolution.
    # ------------------------------------------------------------------
    declare -A PAGE_FILE_MAP=(
        ["UsageGuide"]="index.md"
        ["CreatingSettings"]="creating-settings.md"
        ["CreatingSynth"]="creating-synth.md"
        ["LoadingSoundfonts"]="loading-soundfonts.md"
        ["CreatingAudioDriver"]="audio-driver.md"
        ["UsingSynth"]="using-synth.md"
        ["SendingMIDI"]="sending-midi.md"
        ["RealtimeMIDI"]="realtime-midi.md"
        ["MIDIPlayer"]="midi-player.md"
        ["FileRenderer"]="file-renderer.md"
        ["MIDIPlayerMem"]="midi-player-mem.md"
        ["MIDIRouter"]="midi-router.md"
        ["Sequencer"]="sequencer.md"
        ["Shell"]="shell.md"
        ["Multi-channel"]="multi-channel.md"
        ["synth-context"]="synth-context.md"
        ["Advanced"]="advanced.md"
    )

    # Pages in EXTRA_PAGE_FILE_MAP are only processed when PAGE_FILTER matches them.
    # They are always added to the refmap so sibling pages can link to them.
    declare -A EXTRA_PAGE_FILE_MAP=(
        ["RecentChanges"]="recent-changes.md"
    )

    # ------------------------------------------------------------------
    # Build cross-reference map (same structure as doxy mode):
    #  - API group members  →  ${API_PREFIX}<group-file>.md#<member>
    #  - Page IDs           →  relative path to sibling page .md
    # ------------------------------------------------------------------
    echo "  Building cross-reference map for pages mode ..."
    REFMAP_FILE="$(mktemp pages_refmap.XXXXXX)"
    trap 'rm -f "$REFMAP_FILE"' EXIT

    # Inline XSL to list group compound refids from index.xml
    LIST_GROUPS_XSL='<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text"/>
  <xsl:template match="/">
    <xsl:for-each select="//compound[@kind=&apos;group&apos;]">
      <xsl:value-of select="@refid"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:template>
</xsl:stylesheet>'

    # Inline XSL to extract "id TAB name" pairs from a group XML.
    # Outputs both memberdef entries AND enumvalue entries (the latter mapped to
    # their parent enum type name so they link to the correct heading anchor).
    LIST_MEMBERS_XSL='<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text"/>
  <xsl:template match="/">
    <xsl:for-each select="//memberdef">
      <xsl:value-of select="@id"/>
      <xsl:text>&#9;</xsl:text>
      <xsl:value-of select="name"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
    <xsl:for-each select="//enumvalue">
      <xsl:value-of select="@id"/>
      <xsl:text>&#9;</xsl:text>
      <xsl:value-of select="parent::memberdef/name"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:template>
</xsl:stylesheet>'

    # Collect group compound refids and build refmap with API_PREFIX
    GROUP_REFIDS_PAGES=$(echo "$LIST_GROUPS_XSL" | xsltproc - "$INDEX_XML")

    while IFS= read -r refid; do
        [[ -z "$refid" ]] && continue
        group_xml="$XML_DIR/${refid}.xml"
        [[ -f "$group_xml" ]] || continue

        grp_filename=$(echo "$refid" \
            | sed 's/^group__//' \
            | sed 's/__/-/g')
        grp_link="${API_PREFIX}${grp_filename}.md"

        # Map the group compound itself
        printf '%s|%s\n' "$refid" "$grp_link" >> "$REFMAP_FILE"

        # Extract member pairs and append "member_id|${API_PREFIX}filename#name"
        while IFS=$'\t' read -r member_id member_name; do
            [[ -z "$member_id" ]] && continue
            printf '%s|%s#%s\n' "$member_id" "$grp_link" "$member_name" >> "$REFMAP_FILE"
        done < <(echo "$LIST_MEMBERS_XSL" | xsltproc - "$group_xml")
    done <<< "$GROUP_REFIDS_PAGES"

    # Add cross-page references among the pages themselves.
    # Keys are Doxygen page IDs; values are paths relative to OUTPUT_DIR.
    # Both PAGE_FILE_MAP and EXTRA_PAGE_FILE_MAP contribute so that links between
    # usage pages and recent-changes are properly resolved.
    for page_id in "${!PAGE_FILE_MAP[@]}"; do
        printf '%s|%s\n' "$page_id" "${PAGE_FILE_MAP[$page_id]}" >> "$REFMAP_FILE"
    done
    for page_id in "${!EXTRA_PAGE_FILE_MAP[@]}"; do
        printf '%s|%s\n' "$page_id" "${EXTRA_PAGE_FILE_MAP[$page_id]}" >> "$REFMAP_FILE"
    done

    # Add settings page references (relative from output_dir to settings/)
    # When API_PREFIX ends with "api/" (e.g. "../api/"), strip "api/" to get the parent.
    # When API_PREFIX is "./" (we are already inside api/), settings/ is one level up.
    if [[ "$API_PREFIX" == *"api/" ]]; then
        SETTINGS_PREFIX="${API_PREFIX%api/}settings/"
    else
        SETTINGS_PREFIX="../settings/"
    fi
    printf '%s|%s\n' "fluidsettings" "${SETTINGS_PREFIX}index.md" >> "$REFMAP_FILE"

    # Add individual \setting{} cross-references from fluidsettings.xml if provided.
    # See the doxy mode block above for a full explanation of the refid format.
    if [[ -n "$FLUIDSETTINGS_XML" ]] && [[ -f "$FLUIDSETTINGS_XML" ]]; then
        echo "  Adding settings cross-references from fluidsettings.xml ..."
        while IFS=$'\t' read -r grp raw_name; do
            [[ -z "$grp" ]] && continue
            translated="$(echo "$raw_name" | tr '.' '_')"
            section_id="settings_${grp}_${translated}"
            doxy_refid="settings_${grp}_1${section_id}"
            printf '%s|%s%s.md#%s\n' "$doxy_refid" "$SETTINGS_PREFIX" "$grp" "$section_id" >> "$REFMAP_FILE"
        done < <(xsltproc - "$FLUIDSETTINGS_XML" <<'SETTINGS_XSL'
<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text"/>
  <xsl:template match="/">
    <xsl:for-each select="//setting">
      <xsl:value-of select="name(..)"/>
      <xsl:text>&#9;</xsl:text>
      <xsl:value-of select="name"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
  </xsl:template>
</xsl:stylesheet>
SETTINGS_XSL
        )
        # Add page-level entries for \ref settings_GROUP references
        while IFS= read -r grp; do
            [[ -z "$grp" ]] && continue
            printf 'settings_%s|%s%s.md\n' "$grp" "$SETTINGS_PREFIX" "$grp" >> "$REFMAP_FILE"
        done < <(echo "$FLUIDSETTINGS_GROUPS_XSL" | xsltproc - "$FLUIDSETTINGS_XML")
    fi

    PAGES_REFMAP_TEXT="$(cat "$REFMAP_FILE")"

    # ------------------------------------------------------------------
    # Convert each page XML to Markdown.
    # If PAGE_FILTER is set, only convert that single page ID
    # (searched in both PAGE_FILE_MAP and EXTRA_PAGE_FILE_MAP).
    # If no filter, process all pages in PAGE_FILE_MAP (not EXTRA_PAGE_FILE_MAP).
    # ------------------------------------------------------------------
    echo "  Converting page XML files ..."

    _convert_page() {
        local page_id="$1"
        local out_filename="$2"
        local page_xml="$XML_DIR/${page_id}.xml"
        if [[ ! -f "$page_xml" ]]; then
            echo "    WARNING: page XML not found for ${page_id}: ${page_xml}" >&2
            return 0
        fi
        local out_file="$OUTPUT_DIR/${out_filename}"
        xsltproc \
            --stringparam refmap-text "$PAGES_REFMAP_TEXT" \
            "$XSL_FILE" "$page_xml" > "$out_file"
        echo "    ${page_id} → ${out_filename}"
    }

    if [[ -n "$PAGE_FILTER" ]]; then
        # Filter: look up the page ID in both maps
        if [[ -n "${PAGE_FILE_MAP[$PAGE_FILTER]+_}" ]]; then
            _convert_page "$PAGE_FILTER" "${PAGE_FILE_MAP[$PAGE_FILTER]}"
        elif [[ -n "${EXTRA_PAGE_FILE_MAP[$PAGE_FILTER]+_}" ]]; then
            _convert_page "$PAGE_FILTER" "${EXTRA_PAGE_FILE_MAP[$PAGE_FILTER]}"
        else
            echo "    WARNING: page ID '$PAGE_FILTER' not found in page maps" >&2
        fi
    else
        # No filter: process all usage pages (PAGE_FILE_MAP only, not EXTRA)
        for page_id in "${!PAGE_FILE_MAP[@]}"; do
            _convert_page "$page_id" "${PAGE_FILE_MAP[$page_id]}"
        done
    fi

    echo "  Done. Page docs written to $OUTPUT_DIR/"
    exit 0
fi

# ---------------------------------------------------------------------------
# MODE: examples  (doc/examples/ → api/examples.md + api/examples/<name>.md)
# ---------------------------------------------------------------------------
if [[ "$MODE" = "examples" ]]; then
    EXAMPLES_SRC="$INPUT"
    API_DIR="$OUTPUT_DIR"

    if [[ ! -d "$EXAMPLES_SRC" ]]; then
        echo "ERROR: examples directory not found: $EXAMPLES_SRC" >&2
        exit 1
    fi

    mkdir -p "$API_DIR/examples"

    # --- Write index page linking to each individual example page -----------
    {
        printf "# Code Examples\n\n"
        printf "The following self-contained example programs demonstrate how to use the FluidSynth API.\n\n"

        for src in "$EXAMPLES_SRC"/*.c "$EXAMPLES_SRC"/*.cxx; do
            [[ -f "$src" ]] || continue
            base=$(basename "$src")
            name="${base%.*}"
            # Extract brief description from first comment line starting with capital letter
            desc=$(grep -m1 '^ \* [A-Z]' "$src" | sed 's/^ \* //' || true)
            if [[ -n "$desc" ]]; then
                printf -- "- [**%s**](%s.md) \xe2\x80\x93 %s\n" "$name" "$name" "$desc"
            else
                printf -- "- [**%s**](%s.md)\n" "$name" "$name"
            fi
        done
    } > "$API_DIR/examples/index.md"

    # --- Write one page per example file ------------------------------------
    for src in "$EXAMPLES_SRC"/*.c "$EXAMPLES_SRC"/*.cxx; do
        [[ -f "$src" ]] || continue
        base=$(basename "$src")
        ext="${base##*.}"
        name="${base%.*}"
        lang="c"
        [[ "$ext" = "cxx" ]] && lang="cpp"
        desc=$(grep -m1 '^ \* [A-Z]' "$src" | sed 's/^ \* //' || true)

        {
            printf "# %s\n\n" "$name"
            if [[ -n "$desc" ]]; then
                printf "%s\n\n" "$desc"
            fi
            printf '```%s\n' "$lang"
            cat "$src"
            printf '\n```\n'
        } > "$API_DIR/examples/${name}.md"
        echo "    → examples/${name}.md"
    done

    echo "  Examples pages written to $API_DIR/examples/"
    exit 0
fi

usage

