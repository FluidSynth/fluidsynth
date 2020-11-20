<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" omit-xml-declaration="yes" indent="no"/>

<!-- main template -->
<xsl:template match="/fluidsettings">
/*!
\page fluidsettings Settings Reference
<xsl:apply-templates match="*" mode="PageRef">
    <xsl:sort select="@label"/>
</xsl:apply-templates>

<xsl:apply-templates match="*" mode="Page">
    <xsl:sort select="@label"/>
</xsl:apply-templates>
*/
</xsl:template>


<!-- Page reference template -->
<xsl:template match="*" mode="PageRef">
- \subpage <xsl:value-of select="concat('settings_', name(.), ' ', @label)"/>
</xsl:template>


<!-- Page template -->
<xsl:template match="*" mode="Page">
\page <xsl:value-of select="concat('settings_', name(.), ' ', @label)"/>
<xsl:apply-templates match="*" mode="Setting">
    <xsl:sort select="name(..)" />
</xsl:apply-templates>
</xsl:template>


<!-- Setting template -->
<xsl:template match="*" mode="Setting">
\htmlonly
<div class="setting-item">
\endhtmlonly
\section <xsl:value-of select="concat('settings_', name(..), '_', translate(name, '.', '_'))" /><xsl:text> </xsl:text><xsl:value-of select="concat(name(..), '.', name)" />
<xsl:text>&#xa;</xsl:text>
\par Type
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
    <xsl:otherwise><xsl:value-of select="type" /></xsl:otherwise>
</xsl:choose>

<xsl:choose>
    <xsl:when test="type = 'str' and vals">\par Options
        <xsl:value-of select="vals" /></xsl:when>
    <xsl:when test="type = 'bool'">\par Values
        0, 1</xsl:when>
    <xsl:when test="min or max">\par Min - Max
        <xsl:value-of select="min" /> - <xsl:value-of select="max" /></xsl:when>
</xsl:choose>

\par Default
\htmlonly
<xsl:copy-of select="def" />
\endhtmlonly

<xsl:if test="realtime">
\par Real-time
<xsl:choose>
    <xsl:when test="realtime/text()">
\htmlonly
<xsl:copy-of select="realtime" />
\endhtmlonly
</xsl:when>
    <xsl:otherwise>This setting can be changed during runtime of the synthesizer.</xsl:otherwise>
</xsl:choose>
</xsl:if>

<xsl:if test="deprecated">
\deprecated This setting is deprecated and might be removed in a future version of FluidSynth.
</xsl:if>

\htmlonly
<xsl:copy-of select="desc"/>
\endhtmlonly
\htmlonly
</div>
\endhtmlonly
</xsl:template>

</xsl:stylesheet>
