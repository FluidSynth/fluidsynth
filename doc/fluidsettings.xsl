<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
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

.audio {background-color: hsl(0, 100%, 94%);}
.midi {background-color: hsl(165, 100%, 85%);}
.player {background-color: hsl(60, 100%, 80%);}
.shell {background-color: hsl(36, 100%, 85%);}
.synth {background-color: hsl(105, 100%, 85%);}
            </style>
         </head>
         <body>
            <h2>FluidSettings</h2>
            <table>               
               <!--print each and every setting row by row in the table-->
               <xsl:for-each select="fluidsettings/*/*">
                  <xsl:sort select="name(..)" />
                  <xsl:sort select="name" />
                  <tr>
                     <!-- the class attribute of tr shall be the name of the settings group of the current setting -->
                     <xsl:attribute name="class">
                        <xsl:value-of select="name(..)" />
                     </xsl:attribute>
                     
                     <td class="cell-name first-row">
                        <xsl:attribute name="id"><xsl:value-of select="name(..)" />-<xsl:value-of select="name" /></xsl:attribute>
                        <a>
                            <xsl:attribute name="href">#<xsl:value-of select="name(..)" />-<xsl:value-of select="name" /></xsl:attribute>
                            <xsl:value-of select="name(..)" />.<xsl:value-of select="name" />
                        </a>
                     </td>
                    
                    <th class="first-row">Type</th>
                    
                     <td class="cell-type first-row">
                        <xsl:value-of select="type" />
                     </td>
                  </tr>
                  
                  <tr>
                     <xsl:attribute name="class">
                        <xsl:value-of select="name(..)" />
                     </xsl:attribute>
                     <td></td>
                     <th>Default</th>
                     <td class="cell-def">
                        <xsl:copy-of select="def" />
                     </td>
                  </tr>
                  
                  <tr>
                     <xsl:attribute name="class">
                        <xsl:value-of select="name(..)" />
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
                              1, "yes", 0, "no"
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
                        <xsl:value-of select="name(..)" />
                     </xsl:attribute>
                     <td></td>
                     <th>Description</th>
                     <td class="cell-desc">
                        <xsl:copy-of select="desc" />
                     </td>
                  </tr>
               </xsl:for-each>
            </table>
         </body>
      </html>
   </xsl:template>
</xsl:stylesheet>

