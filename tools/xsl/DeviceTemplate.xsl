<?xml version='1.0'?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:str="http://exslt.org/strings"
	xmlns:d="urn:schemas-upnp-org:device-1-0">
<xsl:import href="common.xsl"/>
<xsl:output method="text" />

<xsl:template match="d:device">
<xsl:variable name="controlClass"><xsl:call-template name="control-class"/></xsl:variable>
<xsl:variable name="templateClass"><xsl:call-template name="template-class"/></xsl:variable>
<xsl:call-template name="file-template"/>
<xsl:for-each select="d:serviceList/d:service">
<xsl:text/>#include &lt;Network/UPnP/<xsl:call-template name="header-template-path"/>&gt;
</xsl:for-each>
<xsl:text/>#include "<xsl:value-of select="$controlClass"/>.h"
<xsl:call-template name="namespace-open"/>
class <xsl:value-of select="$templateClass"/>: public Device
{
public:
	using Device::Device;

	const ObjectClass&amp; getClass() const override
	{
		return <xsl:value-of select="$controlClass"/>_class;
	}
};
<xsl:call-template name="namespace-close"/>
</xsl:template>

</xsl:stylesheet>
