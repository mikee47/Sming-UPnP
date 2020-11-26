<?xml version='1.0'?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:str="http://exslt.org/strings"
	xmlns:s="urn:schemas-upnp-org:service-1-0">
<xsl:import href="common.xsl"/>
<xsl:output method="text" />

<!--
	NOTE: Service schema do not normally contain 'serviceType', so the scanner inserts it for our convenience.
 -->

<xsl:template match="s:scpd">
<xsl:variable name="controlClass"><xsl:call-template name="control-class"/></xsl:variable>
<xsl:variable name="templateClass"><xsl:call-template name="template-class"/></xsl:variable>
<xsl:call-template name="file-template"/>
<xsl:text/>#include &lt;Network/UPnP/ActionResult.h>
#include "<xsl:value-of select="$controlClass"/>.h"
<xsl:call-template name="namespace-open"/>
class <xsl:value-of select="$templateClass"/>: public Service
{
public:
	using Service::Service;

	const ObjectClass&amp; getClass() const override
	{
		return <xsl:value-of select="$controlClass"/>::class_;
	}
};
<xsl:call-template name="namespace-close"/>
</xsl:template>

</xsl:stylesheet>
