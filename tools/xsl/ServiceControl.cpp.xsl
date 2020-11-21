<?xml version='1.0'?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:str="http://exslt.org/strings"
	xmlns:s="urn:schemas-upnp-org:service-1-0">
<xsl:import href="common.xsl"/>
<xsl:output method="text" />

<xsl:template match="s:scpd">
<xsl:call-template name="file-cpp"/>
namespace UPnP {
namespace <xsl:call-template name="urn-domain-cpp"/> {

extern const ClassGroup classGroup;

namespace <xsl:call-template name="urn-kind"/> {

<xsl:variable name="controlClass"><xsl:call-template name="control-class"/></xsl:variable>

DEFINE_FSTR_LOCAL(type, "<xsl:call-template name="urn-type"/>")

const ServiceClass <xsl:value-of select="$controlClass"/>::class_ PROGMEM = {
	classGroup,
	type,
	<xsl:call-template name="urn-version"/>,
	Urn::Kind::service,
	<xsl:call-template name="control-class"/>::createObject
};

<xsl:for-each select="s:actionList/s:action">

<xsl:if test="s:argumentList/s:argument[s:direction='out']">
size_t <xsl:value-of select="$controlClass"/>::<xsl:call-template name="action-result"/>::printTo(Print&amp; p)
{
	size_t n{0};
	<xsl:for-each select="s:argumentList/s:argument[s:direction='out']">
	n += p.print(_F("<xsl:call-template name="varname"/> = "));
	n += p.println(<xsl:call-template name="varname-cpp"/>);
	</xsl:for-each>
	return n;
}
</xsl:if>


bool <xsl:value-of select="$controlClass"/>::<xsl:apply-templates select="." mode="method"/>
{
	<!-- Build request and send it, using a lambda wrapper for response handling -->
	ActionInfo request(*this);
	request.createRequest(_F("<xsl:value-of select="s:name"/>"));<xsl:text/>
	<xsl:for-each select="s:argumentList/s:argument[s:direction='in']">
	request.addArg(_F("<xsl:value-of select="s:name"/>"), <xsl:call-template name="varname-cpp"/>);<xsl:text/>
	</xsl:for-each>
	return sendRequest(request, [callback](UPnP::ActionInfo&amp; response) {
		<!-- Process response and invoke user callback -->
		<xsl:choose>
		<xsl:when test="s:argumentList/s:argument[s:direction='out']">
		<xsl:call-template name="action-result"/> result;
		<xsl:for-each select="s:argumentList/s:argument[s:direction='out']">
		response.getArg(_F("<xsl:value-of select="s:name"/>"), result.<xsl:call-template name="varname-cpp"/>);<xsl:text/>
		</xsl:for-each>
		callback(result);
		</xsl:when>
		<xsl:otherwise>
		callback();
		</xsl:otherwise>
		</xsl:choose>
	});
}
</xsl:for-each>

<xsl:call-template name="namespace-close"/>

</xsl:template>



</xsl:stylesheet>
