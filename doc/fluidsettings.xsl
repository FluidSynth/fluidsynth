<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
    <xsl:output method="html" doctype-system="about:legacy-compat"/>
    <xsl:template match="/">
        <html>
            <head>
                <style>
body {
    margin: 0;
    padding: 0;
    font-family: sans-serif;
    background: #eee;
}

#sidebar {
    position: fixed;
    width: 25em;
    top: 0;
    bottom: 0;
    padding-bottom: 2em;
    box-sizing: border-box;
    overflow-y: auto;
    color: white;
    background: #333;
}

#sidebar ul li a {
    display: block;
    padding-left: 5%;
    padding-top: 0.3em;
    padding-bottom: 0.3em;
    color: #fafafa;
}

#sidebar a:hover {
    background: #666;
}

#sidebar .muted {
    color: #ccc !important;
}

#sidebar h1 {
    padding-top: 1em;
    margin: 0;
    padding-left: 5%;
}

#sidebar h2 {
    padding-left: 5%;
    margin-top: 1.5em;
    margin-bottom: 0.5em;
    color: lightblue;
}

#sidebar ul,
#sidebar ul li
{
    list-style: none;
    margin: 0;
    padding: 0;
}

#sidebar li a {
    text-decoration: none;
}

.deprecated-badge {
    margin-left: 0.5em;
    font-size: 80%;
    font-weight: bold;
    color: red;
}

#main {
    margin-left: 25em;
    padding: 1em 2em;
    box-sizing: border-box;
    max-width: 60em;
    background: white;
    color: #333;
}

#main h2 {
    margin-top: 2em;
}

#main h2:first-child {
    margin-top: 1em;
}

.setting {
    padding: 0;
    margin: 1em 0;
    border-left: 1px solid #eee;
    border-radius: 5px 0px 0px 0px;
}

@-webkit-keyframes flash-header {
    from { background: lightblue; }
    50% { background: #eee; }
    to { background: lightblue; }
}

@keyframes flash-header {
    from { background: lightblue; }
    50% { background: #eee; }
    to { background: lightblue; }
}

.setting:target .setting-header {
    background: lightblue;
    -webkit-animation: flash-header .5s 2 linear;
    animation: flash-header .5s 2 linear;
}

.setting-header {
    background: #eee;
    border-radius: 5px 0px 0px 0px;
    padding: 0.5em 1em;
    position: relative;
}

.setting-body {
    padding: 1em;
}

.setting-name {
    font-weight: bold;
    display: inline;
    color: #333;
}

.setting-type {
    color: #666;
    font-weight: bold;
    float: right;
}

.setting-attribute {
    margin-bottom: 0.8em;
    color: #666;
}

.setting-attribute .label {
    display: inline-block;
    vertical-align: top;
    min-width: 6em;
}

.setting-attribute .value {
    display: inline-block;
    color: #333;
}

.setting-deprecated {
    color: red;
}

.setting-description {
    color: black;
    margin-top: 1.5em;
}

a {
    color: darkblue;
}
                </style>
                <title>FluidSynth Settings</title>
            </head>
            <body>
                <div id="sidebar">
                    <h1>FluidSynth Settings</h1>

                    <xsl:for-each select="fluidsettings/*">
                        <xsl:sort select="@label" />

                        <h2><xsl:value-of select="@label" /></h2>
                        <ul>
                            <xsl:for-each select="*">
                                <li>
                                    <a>
                                        <xsl:attribute name="href"><![CDATA[#]]><xsl:value-of select="name(..)" /><![CDATA[.]]><xsl:value-of select="name" /></xsl:attribute>
                                        <span class="muted"><xsl:value-of select="name(..)" /></span>.<xsl:value-of select="name" />
                                        <xsl:if test="deprecated">
                                            <span class="deprecated-badge">deprecated</span>
                                        </xsl:if>
                                    </a>
                                </li>
                            </xsl:for-each>
                        </ul>
                    </xsl:for-each>
                </div>

                <div id="main">
                    <xsl:for-each select="fluidsettings/*">
                        <xsl:sort select="@label" />

                        <h2><xsl:value-of select="@label" /></h2>

                        <xsl:for-each select="*">
                            <xsl:sort select="name(..)" />
                            <div class="setting">
                                <xsl:attribute name="id">
                                    <xsl:value-of select="name(..)" /><![CDATA[.]]><xsl:value-of select="name" />
                                </xsl:attribute>

                                <div class="setting-header">
                                    <div class="setting-name">
                                        <xsl:value-of select="name(..)" />.<xsl:value-of select="name" />
                                    </div>

                                    <div class="setting-type">
                                    </div>
                                </div>

                                <div class="setting-body">

                                    <div class="setting-attribute">
                                        <div class="label">Type:</div>
                                        <div class="value">
                                            <xsl:choose>
                                                <xsl:when test="type = 'bool'">
                                                    Boolean (int)
                                                </xsl:when>
                                                <xsl:when test="type = 'int'">
                                                    Integer (int)
                                                </xsl:when>
                                                <xsl:when test="type = 'str'">
                                                    <xsl:choose>
                                                        <xsl:when test="vals">
                                                            Selection (str)
                                                        </xsl:when>
                                                        <xsl:otherwise>
                                                            String (str)
                                                        </xsl:otherwise>
                                                    </xsl:choose>
                                                </xsl:when>
                                                <xsl:when test="type = 'num'">
                                                    Float (num)
                                                </xsl:when>
                                                <xsl:otherwise>
                                                    <xsl:value-of select="type" />
                                                </xsl:otherwise>
                                            </xsl:choose>
                                        </div>
                                    </div>

                                    <xsl:choose>
                                        <xsl:when test="type = 'str' and vals">
                                            <div class="setting-attribute">
                                                <div class="label">Options:</div>
                                                <div class="value"><xsl:value-of select="vals" /></div>
                                            </div>
                                        </xsl:when>
                                        <xsl:when test="type = 'bool'">
                                            <div class="setting-attribute">
                                                <div class="label">Values:</div>
                                                <div class="value">0, 1</div>
                                            </div>
                                        </xsl:when>
                                        <xsl:when test="min or max">
                                            <div class="setting-attribute">
                                                <div class="label">Min - Max:</div>
                                                <div class="value">
                                                    <xsl:value-of select="min" />
                                                    -
                                                    <xsl:value-of select="max" />
                                                </div>
                                            </div>
                                        </xsl:when>
                                    </xsl:choose>

                                    <div class="setting-attribute">
                                        <div class="label">Default:</div>
                                        <div class="value"><xsl:copy-of select="def" /></div>
                                    </div>

                                    <xsl:if test="realtime">
                                        <div class="setting-attribute">
                                            <div class="label">Real-time:</div>
                                            <div class="value">
                                                <xsl:choose>
                                                    <xsl:when test="realtime/text()">
                                                        <xsl:copy-of select="realtime"/>
                                                    </xsl:when>
                                                    <xsl:otherwise>
                                                        This setting can be changed during runtime of the synthesizer.
                                                    </xsl:otherwise>
                                                </xsl:choose>
                                            </div>
                                        </div>
                                    </xsl:if>

                                    <xsl:if test="deprecated">
                                        <div class="setting-deprecated">
                                            This setting is deprecated and might be removed in a future version of FluidSynth.
                                        </div>
                                    </xsl:if>

                                    <div class="setting-description">
                                        <xsl:copy-of select="desc" />
                                    </div>
                                </div>
                            </div>
                        </xsl:for-each>
                    </xsl:for-each>
                </div>
            </body>
        </html>
    </xsl:template>
</xsl:stylesheet>
