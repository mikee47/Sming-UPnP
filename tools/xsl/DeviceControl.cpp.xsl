<?xml version='1.0'?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:str="http://exslt.org/strings"
	xmlns:d="urn:schemas-upnp-org:device-1-0">
<xsl:import href="common.xsl"/>
<xsl:output method="text"/>

<xsl:template match="d:device">
<xsl:variable name="controlClass"><xsl:call-template name="control-class"/></xsl:variable>
<xsl:call-template name="file-cpp"/>

namespace UPnP {
namespace <xsl:call-template name="urn-domain-cpp"/> {

namespace <xsl:call-template name="urn-kind"/> {

<xsl:call-template name="namespace-close"/>

</xsl:template>

</xsl:stylesheet>
