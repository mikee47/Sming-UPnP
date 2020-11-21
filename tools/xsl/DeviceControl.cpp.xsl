<?xml version='1.0'?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:str="http://exslt.org/strings"
	xmlns:d="urn:schemas-upnp-org:device-1-0">
<xsl:import href="common.xsl"/>
<xsl:output method="text" />
<xsl:param name="NS1"/>
<xsl:param name="NS2"/>

<xsl:template match="d:device">
<xsl:call-template name="file-cpp"/>

namespace UPnP {
namespace <xsl:call-template name="urn-domain-cpp"/> {

extern const ClassGroup classGroup;

namespace <xsl:call-template name="urn-kind"/> {

DEFINE_FSTR_LOCAL(type, "<xsl:call-template name="urn-type"/>");

const DeviceClass <xsl:call-template name="control-class"/>::class_ PROGMEM = {
	classGroup,
	type,
	<xsl:call-template name="urn-version"/>,
	Urn::Kind::device,
	<xsl:call-template name="control-class"/>::createObject
};


<xsl:call-template name="namespace-close"/>

</xsl:template>

</xsl:stylesheet>
