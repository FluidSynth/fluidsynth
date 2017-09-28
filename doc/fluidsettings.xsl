<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
   <xsl:template match="/">
      <html>
         <head>
            <style>
.int  {background-color: powderblue;}
.bool {background-color: lime;}
.num  {background-color: yellow;}
.str  {background-color: red;}

.audio {background-color: rgb( 255, 190, 170 );}
.midi {background-color: rgb( 200, 255, 240 );}
.player {background-color: rgb( 255, 255, 180 );}
.synth {background-color: rgb( 190, 255, 170 );}
            </style>
         </head>
         <body>
            <h2>FluidSettings</h2>
            <table border="1">
               <tr bgcolor="#9acd32">
                  <th>Name</th>
                  <th>Type</th>
                  <th>Default Value</th>
                  <th>Allowed Values</th>
                  <th>Description</th>
               </tr>
               
               <!--print each and every setting to its own row in the table-->
               <xsl:for-each select="fluidsettings/*/*">
                  <xsl:sort select=".." />
                  <xsl:sort select="name" />
                  <tr>
                     <!-- the class attribute of tr shall be the name of the settings group of the current setting -->
                     <xsl:attribute name="class">
                        <xsl:value-of select="name(..)" />
                     </xsl:attribute>
                     
                     <td>
                        <xsl:value-of select="name(..)" />.<xsl:value-of select="name" />
                     </td>
                     <xsl:choose>
                        <xsl:when test="type = 'Integer'">
                           <td style="text-align:center">
                              <xsl:value-of select="type" />
                           </td>
                        </xsl:when>
                        <xsl:otherwise>
                           <td style="text-align:center">
                              <xsl:value-of select="type" />
                           </td>
                        </xsl:otherwise>
                     </xsl:choose>
                     <td style="text-align:center">
                        <xsl:value-of select="def" />
                     </td>
                     <td style="text-align:center">
                        <xsl:value-of select="vals" />
                     </td>
                     <td>
                        <xsl:value-of select="desc" />
                     </td>
                  </tr>
               </xsl:for-each>
            </table>
         </body>
      </html>
   </xsl:template>
</xsl:stylesheet>

