<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
   <xsl:template match="/">
      <html>
         <head>
            <style>
table
{
    border: 2px solid black;
}
            
td
{
<!--      border-bottom: 2px solid black; -->
<!--      border-top: 0px solid black; -->
<!--      padding-top: 20px; -->
<!--      padding-bottom: 20px; -->
    padding: 15px 5px 15px 5px;
}

.cell-type
{
    text-align: center;
    white-space: nowrap;
    border-top: 2px solid black;
}
.cell-def
{
    text-align: center;
    white-space: nowrap;
    border-top: 2px solid black;
}
.cell-vals
{
    text-align: center;
    border-top: 2px solid black;
}
.cell-name { border-top: 2px solid black; }

.audio {background-color: hsl(0, 100%, 90%);}
.midi {background-color: hsl(165, 100%, 85%);}
.player {background-color: hsl(60, 100%, 80%);}
.shell {background-color: hsl(36, 100%, 85%);}
.synth {background-color: hsl(105, 100%, 85%);}
            </style>
         </head>
         <body>
            <h2>FluidSettings</h2>
            <table>
               <tr bgcolor="#9acd32">
                  <th>Name</th>
                  <th>Type</th>
                  <th>Default Value</th>
                  <th>Allowed Values</th>
<!--                   <th>Description</th> -->
               </tr>
               
               <!--print each and every setting to its own row in the table-->
               <xsl:for-each select="fluidsettings/*/*">
                  <xsl:sort select="name(..)" />
                  <xsl:sort select="name" />
                  <tr>
                     <!-- the class attribute of tr shall be the name of the settings group of the current setting -->
                     <xsl:attribute name="class">
                        <xsl:value-of select="name(..)" />
                     </xsl:attribute>
                     
                     <td class="cell-name">
                        <xsl:value-of select="name(..)" />.<xsl:value-of select="name" />
                     </td>
                    
                     <td class="cell-type">
                        <xsl:value-of select="type" />
                     </td>
                        
                     <td class="cell-def">
                        <xsl:copy-of select="def" />
                     </td>
                     
                     <td class="cell-vals">
                     <xsl:choose>
                        <xsl:when test="type = 'str'">
                              <xsl:value-of select="vals" />
                        </xsl:when>
                        <xsl:when test="type = 'bool'">
                              
                        </xsl:when>
                        <xsl:otherwise>
                              [
                              <xsl:value-of select="min" />
                              ;
                              <xsl:value-of select="max" />
                              ]
                        </xsl:otherwise>
                     </xsl:choose>
                     </td>
                   </tr>
                   <tr>
                     <xsl:attribute name="class">
                        <xsl:value-of select="name(..)" />
                     </xsl:attribute>
                     <td class="cell-desc" colspan="4">
                        <xsl:copy-of select="desc" />
                     </td>
                  </tr>
               </xsl:for-each>
            </table>
         </body>
      </html>
   </xsl:template>
</xsl:stylesheet>

