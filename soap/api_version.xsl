<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                              xmlns:soap="http://schemas.xmlsoap.org/wsdl/soap/"
			      xmlns:wns="http://schemas.xmlsoap.org/wsdl/"
>
<!-- Code to extract the Force.com API version from a Partner WSDL -->

<xsl:output method="text" omit-xml-declaration="yes" />

<!-- top level match for root -->
<xsl:template match="/">
  <xsl:apply-templates select="/wns:definitions/wns:service/wns:port/soap:address"/>
</xsl:template>

<!-- look for soap address URL -->
<xsl:template match="/wns:definitions/wns:service/wns:port/soap:address">
  <!-- send to XSLT 1.0 path cracker.  2.0 has tokenize... -->
  <xsl:call-template name="basepath">
    <xsl:with-param name="path">
      <!-- the sole parameter is the location= attribute from soap:address -->
      <xsl:value-of select="@location"/>
    </xsl:with-param>
  </xsl:call-template>
</xsl:template>

<xsl:template name="basepath">
  <xsl:param name="path" />
  <!-- find part after the first / and recurse -->
  <xsl:variable name="remaining" select="substring-after($path,'/')" />
  <xsl:if test="not($remaining)" >
    <!-- if nothing after first slash (or no slash at all), we are done; use input value -->
    <xsl:value-of select="$path" />
    <xsl:text>&#xa;</xsl:text>
  </xsl:if>
  <xsl:if test="$remaining">
    <!-- otherwise recurse using remaining string -->
    <xsl:call-template name="basepath">
      <xsl:with-param name="path">
	<xsl:value-of select="$remaining" />
      </xsl:with-param>
    </xsl:call-template>
  </xsl:if>
</xsl:template>

</xsl:stylesheet>
