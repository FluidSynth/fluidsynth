<?xml version="1.0" encoding="UTF-8"?>
<!--
  fluidsettings2md.xsl
  =====================
  Converts fluidsettings.xml to a set of Markdown files suitable for
  the Zensical / MkDocs documentation site.

  Since XSLT 1.0 writes to a single output stream, this stylesheet
  generates *one combined Markdown file* that contains all groups.
  A thin shell wrapper (run-xslt.sh) is responsible for splitting the
  output into per-group pages using the sentinel lines of the form:

      __FILE__: <filename>

  Usage (via the wrapper):
      xsltproc fluidsettings2md.xsl fluidsettings.xml | \
          run-xslt.sh <output_dir>

  Anchor IDs use the same scheme as the legacy fluidsettings.xsl so that
  \setting{} cross-references in the usage guide remain valid:

      settings_<group>_<name-with-dots-as-underscores>
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text" encoding="UTF-8" omit-xml-declaration="yes"/>

  <!-- ======================================================================
       Root template: emit index page then per-group pages
       ====================================================================== -->
  <xsl:template match="/fluidsettings">
    <!-- index.md -->
    <xsl:text>__FILE__: index.md&#xa;</xsl:text>
    <xsl:text># ⚙️ Fluid Settings&#xa;</xsl:text>
    <xsl:text>
FluidSynth provides numerous options that allow tweaking various aspects of the synthesizing process, midi player and audio drivers. These are referred to as **FluidSettings**. Each setting is handled as a string, while the value this setting can be set to may either be an integer, number (float), bool or string type. They can be either used via [fluidsynth's API](https://www.fluidsynth.org/api/fluidsettings.html) or with the fluidsynth executable like:

```
fluidsynth -o audio.driver=alsa -o audio.alsa.device=plughw:0
```

FluidSynth settings are organised into the following groups. Each setting has a name in *group.name* dotted notation, a type, a default value, and an optional range:&#xa;&#xa;</xsl:text>
    <xsl:apply-templates select="*" mode="index-entry">
      <xsl:sort select="@label"/>
    </xsl:apply-templates>

    <xsl:text>
Starting with **FluidSynth 2.0**, the [**FluidSettings are documented in this XML file**](https://github.com/FluidSynth/fluidsynth/blob/master/doc/fluidsettings.xml). If you want to propose changes, this is the place to look for.

---

For legacy FluidSynth 1.1 pls. refer to FluidSynths man page at that time.&#xa;</xsl:text>

    <!-- per-group pages -->
    <xsl:apply-templates select="*" mode="page">
      <xsl:sort select="@label"/>
    </xsl:apply-templates>
  </xsl:template>

  <!-- ======================================================================
       Index entry
       ====================================================================== -->
  <xsl:template match="*" mode="index-entry">
    <xsl:text>- [</xsl:text>
    <xsl:value-of select="@label"/>
    <xsl:text>](</xsl:text>
    <xsl:value-of select="name(.)"/>
    <xsl:text>.md)&#xa;</xsl:text>
  </xsl:template>

  <!-- ======================================================================
       Per-group page
       ====================================================================== -->
  <xsl:template match="*" mode="page">
    <xsl:variable name="group" select="name(.)"/>
    <xsl:text>&#xa;__FILE__: </xsl:text>
    <xsl:value-of select="$group"/>
    <xsl:text>.md&#xa;</xsl:text>
    <xsl:text># </xsl:text>
    <xsl:value-of select="@label"/>
    <xsl:text>&#xa;</xsl:text>
    <xsl:apply-templates select="setting">
      <xsl:sort select="name"/>
    </xsl:apply-templates>
  </xsl:template>

  <!-- ======================================================================
       Individual setting
       ====================================================================== -->
  <xsl:template match="setting">
    <xsl:variable name="group" select="name(..)"/>
    <xsl:variable name="raw-name" select="name"/>
    <!-- Anchor: replace dots with underscores in the name part -->
    <xsl:variable name="anchor-name">
      <xsl:call-template name="replace-dots">
        <xsl:with-param name="text" select="$raw-name"/>
      </xsl:call-template>
    </xsl:variable>

    <!-- Section heading with explicit anchor -->
    <xsl:text>&#xa;## `</xsl:text>
    <xsl:value-of select="$group"/>
    <xsl:text>.</xsl:text>
    <xsl:value-of select="$raw-name"/>
    <xsl:text>` {#settings_</xsl:text>
    <xsl:value-of select="$group"/>
    <xsl:text>_</xsl:text>
    <xsl:value-of select="$anchor-name"/>
    <xsl:text>}&#xa;&#xa;</xsl:text>

    <!-- Deprecated admonition -->
    <xsl:if test="deprecated">
      <xsl:text>!!! warning "Deprecated"&#xa;    </xsl:text>
      <xsl:call-template name="inline-html">
        <xsl:with-param name="node" select="deprecated"/>
      </xsl:call-template>
      <xsl:text>&#xa;&#xa;</xsl:text>
    </xsl:if>

    <!-- Real-time admonition -->
    <xsl:if test="realtime">
      <xsl:text>!!! tip "Real-time"&#xa;    </xsl:text>
      <xsl:choose>
        <xsl:when test="normalize-space(realtime) != ''">
          <xsl:call-template name="inline-html">
            <xsl:with-param name="node" select="realtime"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>This setting can be changed at run time.</xsl:otherwise>
      </xsl:choose>
      <xsl:text>&#xa;&#xa;</xsl:text>
    </xsl:if>

    <!-- Property table -->
    <xsl:text>| Property | Value |&#xa;</xsl:text>
    <xsl:text>|----------|-------|&#xa;</xsl:text>

    <!-- Type row -->
    <xsl:text>| Type | `</xsl:text>
    <xsl:choose>
        <xsl:when test="type = 'bool'">Boolean (int)</xsl:when>
        <xsl:when test="type = 'int'">Integer (int)</xsl:when>
        <xsl:when test="type = 'str'">
            <xsl:choose>
                <xsl:when test="vals">Selection (str)</xsl:when>
                <xsl:otherwise>String (str)</xsl:otherwise>
            </xsl:choose>
        </xsl:when>
        <xsl:when test="type = 'num'">Float (num)</xsl:when>
        <xsl:otherwise>
            <xsl:value-of select="type" />
        </xsl:otherwise>
    </xsl:choose>
    <xsl:text>` |&#xa;</xsl:text>

    <!-- Default row (may contain HTML markup like <br/>) -->
    <xsl:text>| Default | `</xsl:text>
    <xsl:call-template name="inline-html">
      <xsl:with-param name="node" select="def"/>
      <xsl:with-param name="in-table" select="'1'"/>
    </xsl:call-template>
    <xsl:text>` |&#xa;</xsl:text>

    <!-- Min/Max rows -->
    <xsl:if test="min">
      <xsl:text>| Min | `</xsl:text>
      <xsl:value-of select="min"/>
      <xsl:text>` |&#xa;</xsl:text>
    </xsl:if>
    <xsl:if test="max">
      <xsl:text>| Max | `</xsl:text>
      <xsl:value-of select="max"/>
      <xsl:text>` |&#xa;</xsl:text>
    </xsl:if>

    <!-- Options row for selection strings -->
    <xsl:if test="type = 'str' and vals">
      <xsl:text>| Options | `</xsl:text>
      <xsl:value-of select="vals"/>
      <xsl:text>` |&#xa;</xsl:text>
    </xsl:if>
    <xsl:text>&#xa;</xsl:text>

    <!-- Description -->
    <xsl:if test="normalize-space(desc) != ''">
      <xsl:call-template name="inline-html">
        <xsl:with-param name="node" select="desc"/>
      </xsl:call-template>
      <xsl:text>&#xa;</xsl:text>
    </xsl:if>
  </xsl:template>

  <!-- ======================================================================
       inline-html: serialize child nodes of an element as plain text,
       converting HTML tags to Markdown equivalents.
       in-table: when '1', <br/> is rendered as a space (linebreaks not
       allowed in Markdown table cells); otherwise a Markdown hard line
       break (two trailing spaces + newline) is emitted.
       ====================================================================== -->
  <xsl:template name="inline-html">
    <xsl:param name="node"/>
    <xsl:param name="list-indent" select="'  '"/>
    <xsl:param name="in-table" select="'0'"/>
    <xsl:for-each select="$node/node()">
      <xsl:choose>
        <xsl:when test="self::text()">
          <!-- Normalise leading/trailing whitespace in text nodes -->
          <xsl:value-of select="normalize-space(.)"/>
        </xsl:when>
        <xsl:when test="self::*[name()='br' or name()='BR']">
          <xsl:choose>
            <xsl:when test="$in-table = '1'">
              <!-- Linebreaks inside Markdown table cells must be a space -->
              <xsl:text> </xsl:text>
            </xsl:when>
            <xsl:otherwise>
              <!-- Hard line break outside a table: two trailing spaces + newline -->
              <xsl:text>  &#xa;</xsl:text>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:when>
        <xsl:when test="self::*[name()='code']">
          <xsl:text>`</xsl:text>
          <xsl:value-of select="."/>
          <xsl:text>`</xsl:text>
        </xsl:when>
        <xsl:when test="self::*[name()='ul']">
          <xsl:text>&#xa;&#xa;</xsl:text>
          <xsl:for-each select="li">
            <xsl:value-of select="concat($list-indent, '* ')"/>
            <!-- Recurse for the content of the li, pass increased indent for nested lists -->
            <xsl:call-template name="inline-html">
              <xsl:with-param name="node" select="."/>
              <xsl:with-param name="list-indent" select="concat($list-indent, '  ')"/>
              <xsl:with-param name="in-table" select="$in-table"/>
            </xsl:call-template>
          <xsl:text>&#xa;&#xa;</xsl:text>
          </xsl:for-each>
        </xsl:when>
        <xsl:when test="self::*[name()='li']">
          <xsl:value-of select="concat($list-indent, '* ')"/>
          <xsl:call-template name="inline-html">
            <xsl:with-param name="node" select="."/>
            <xsl:with-param name="list-indent" select="concat($list-indent, '  ')"/>
            <xsl:with-param name="in-table" select="$in-table"/>
          </xsl:call-template>
          <xsl:text>&#xa;</xsl:text>
        </xsl:when>
        <xsl:when test="self::*[name()='strong' or name()='b']">
          <xsl:text>**</xsl:text>
          <xsl:value-of select="."/>
          <xsl:text>**</xsl:text>
        </xsl:when>
        <xsl:when test="self::*[name()='em' or name()='i']">
          <xsl:text>*</xsl:text>
          <xsl:value-of select="."/>
          <xsl:text>*</xsl:text>
        </xsl:when>
        <xsl:when test="self::*[name()='a']">
          <xsl:text>[</xsl:text>
          <xsl:value-of select="."/>
          <xsl:text>](</xsl:text>
          <xsl:value-of select="@href"/>
          <xsl:text>)</xsl:text>
        </xsl:when>
        <xsl:when test="self::*[name()='note']">
          <!-- Zensical admonition: note -->
          <xsl:text>&#xa;&#xa;!!! note&#xa;    </xsl:text>
          <xsl:call-template name="inline-html">
            <xsl:with-param name="node" select="."/>
            <xsl:with-param name="in-table" select="$in-table"/>
          </xsl:call-template>
          <xsl:text>&#xa;</xsl:text>
        </xsl:when>
        <xsl:when test="self::*[name()='attention']">
          <!-- Zensical admonition: warning (attention = stronger notice) -->
          <xsl:text>&#xa;&#xa;!!! warning "Attention"&#xa;    </xsl:text>
          <xsl:call-template name="inline-html">
            <xsl:with-param name="node" select="."/>
            <xsl:with-param name="in-table" select="$in-table"/>
          </xsl:call-template>
          <xsl:text>&#xa;</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <!-- Unknown element: just output its text content -->
          <xsl:value-of select="."/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>

  <!-- ======================================================================
       replace-dots: replace '.' with '_' in a string (XSLT 1.0 approach)
       ====================================================================== -->
  <xsl:template name="replace-dots">
    <xsl:param name="text"/>
    <xsl:choose>
      <xsl:when test="contains($text, '.')">
        <xsl:value-of select="substring-before($text, '.')"/>
        <xsl:text>_</xsl:text>
        <xsl:call-template name="replace-dots">
          <xsl:with-param name="text" select="substring-after($text, '.')"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$text"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>
