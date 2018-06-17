<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
   <xsl:output method="html" doctype-system="about:legacy-compat"/>
   <xsl:template match="/">
      <html>
         <head>
            <style>
table
{
    border: 3px solid black;
}

.first-row
{
    border-top: 3px solid black;
}

th
{
    font-weight: normal;
    white-space: nowrap;
    padding: 15px 5px 15px 5px;
    border-top: 1px solid black;
    border-left: 1px solid black;
    border-right: 1px solid black;
}


td
{
    padding: 15px 5px 15px 5px;
}

.cell-def
{
    white-space: nowrap;
    border-top: 1px solid black;
}
.cell-vals
{
    border-top: 1px solid black;
}
.cell-desc
{
    border-top: 1px solid black;
}

.audio {background-color: hsl(170, 100%, 90%);}
.midi {background-color: hsl(125, 100%, 90%);}
.player {background-color: hsl(85, 100%, 85%);}
.shell {background-color: hsl(60, 100%, 90%);}
.synth {background-color: hsl(35, 100%, 90%);}
.deprecated {background-color: hsl(0, 0%, 93%);}
            </style>
            <title>FluidSettings</title>
         </head>
         <body>
            <h1>FluidSettings</h1>
            
            <ul>
             <!-- Select the first setting of each group and use it for building up a TOC -->
             <xsl:for-each select="fluidsettings/*/*[isFirst]">
               <xsl:sort select="name(..)" />
               <li style="margin-bottom: 15px">
                <xsl:attribute name="class">
                    <xsl:value-of select="name(..)" />
                </xsl:attribute>
                  <a>
                    <xsl:attribute name="href"><![CDATA[#]]><xsl:value-of select="name(..)" /><![CDATA[.]]><xsl:value-of select="name" /></xsl:attribute>
                    <xsl:value-of select="isFirst" />
                  </a>
               </li>
             </xsl:for-each>
            </ul>
            
            
            <table>               
               <!--print each and every setting row by row in the table-->
               <xsl:for-each select="fluidsettings/*/*">
                  <xsl:sort select="name(..)" />
<!--                   <xsl:sort select="name" /> -->
                  <tr>
                     <!-- the class attribute of tr shall be the name of the settings group of the current setting, unless the setting is marked deprecated -->
                     <xsl:attribute name="class">
                        <xsl:choose>
                            <xsl:when test="deprecated">
                                deprecated
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:value-of select="name(..)" />
                            </xsl:otherwise>
                        </xsl:choose>
                     </xsl:attribute>
                     
                     <td class="cell-name first-row">
                        <xsl:attribute name="id"><xsl:value-of select="name(..)" /><![CDATA[.]]><xsl:value-of select="name" /></xsl:attribute>
                        <a>
                            <xsl:attribute name="href"><![CDATA[#]]><xsl:value-of select="name(..)" /><![CDATA[.]]><xsl:value-of select="name" /></xsl:attribute>
                            <xsl:value-of select="name(..)" />.<xsl:value-of select="name" />
                        </a>
                     </td>
                    
                    <th class="first-row">Type</th>
                    
                     <td class="cell-type first-row">
                        <xsl:choose>
                            <xsl:when test="type = 'bool'">
                                int (bool)
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:value-of select="type" />
                            </xsl:otherwise>
                        </xsl:choose>
                     </td>
                  </tr>
                  
                  <tr>
                     <xsl:attribute name="class">
                        <xsl:choose>
                            <xsl:when test="deprecated">
                                deprecated
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:value-of select="name(..)" />
                            </xsl:otherwise>
                            
                        </xsl:choose>
                     </xsl:attribute>
                     
                     <td></td>
                     <th>Default</th>
                     <td class="cell-def">
                        <xsl:copy-of select="def" />
                     </td>
                  </tr>
                  
                  <tr>
                     <xsl:attribute name="class">
                        <xsl:choose>
                            <xsl:when test="deprecated">
                                deprecated
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:value-of select="name(..)" />
                            </xsl:otherwise>
                            
                        </xsl:choose>
                     </xsl:attribute>
                     
                     <td></td>
                     <th>
                     <xsl:choose>
                        <xsl:when test="type = 'str'">
                              Values
                        </xsl:when>
                        <xsl:when test="type = 'bool'">
                              Values
                        </xsl:when>
                        <xsl:otherwise>
                              Min
                              -
                              Max
                        </xsl:otherwise>
                     </xsl:choose>
                     </th>
                     
                     <td class="cell-vals">
                     <xsl:choose>
                        <xsl:when test="type = 'str'">
                              <xsl:value-of select="vals" />
                        </xsl:when>
                        <xsl:when test="type = 'bool'">
                              1, 0
                        </xsl:when>
                        <xsl:otherwise>
                              <xsl:value-of select="min" />
                              -
                              <xsl:value-of select="max" />
                        </xsl:otherwise>
                     </xsl:choose>
                     </td>
                   </tr>
                   
                   <tr>
                     <xsl:attribute name="class">
                        <xsl:choose>
                            <xsl:when test="deprecated">
                                deprecated
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:value-of select="name(..)" />
                            </xsl:otherwise>
                            
                        </xsl:choose>
                     </xsl:attribute>
                     
                     <td></td>
                     <th>Description</th>
                     <td class="cell-desc">
                        <xsl:copy-of select="desc" />
                        <xsl:choose>
                            <xsl:when test="deprecated">
                                <br /><br />
                                <strong style="color:red">DEPRECATED</strong><br /><br />
                                <xsl:copy-of select="deprecated" />
                            </xsl:when>
                        </xsl:choose>
                     </td>
                  </tr>
               </xsl:for-each>
            </table>
         </body>
      </html>
   </xsl:template>
</xsl:stylesheet>

