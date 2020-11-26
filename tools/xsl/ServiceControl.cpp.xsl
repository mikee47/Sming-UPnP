<?xml version='1.0'?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:str="http://exslt.org/strings"
	xmlns:s="urn:schemas-upnp-org:service-1-0">
<xsl:import href="common.xsl"/>
<xsl:output method="text"/>

<xsl:template match="s:scpd">
<xsl:call-template name="file-cpp"/>
namespace UPnP {
namespace <xsl:call-template name="urn-domain-cpp"/> {

extern const ClassGroup classGroup;

namespace <xsl:call-template name="urn-kind"/> {

<xsl:variable name="controlClass"><xsl:call-template name="control-class"/></xsl:variable>

namespace {
DEFINE_FSTR(type, "<xsl:call-template name="urn-type"/>")
IMPORT_FSTR(<xsl:value-of select="$controlClass"/>_schema, COMPONENT_PATH "/schema/<xsl:call-template name="file-path"/>.xml")
}

const ObjectClass <xsl:value-of select="$controlClass"/>::class_ PROGMEM = {
	classGroup,
	type,
	<xsl:value-of select="$controlClass"/>_schema,
	<xsl:call-template name="urn-version"/>,
	Urn::Kind::service,
	<xsl:call-template name="control-class"/>::createObject
};

<xsl:for-each select="s:actionList/s:action">

<xsl:if test="s:argumentList/s:argument[s:direction='out']">
size_t <xsl:value-of select="$controlClass"/>::<xsl:apply-templates select="." mode="name"/>::Result::printTo(Print&amp; p)
{
	size_t n{0};
	<xsl:for-each select="s:argumentList/s:argument[s:direction='out']">
	n += p.print(Arg::<xsl:call-template name="varname"/>);
	n += p.print(" = ");
	n += p.println(<xsl:call-template name="varname-cpp"/>());
	</xsl:for-each>
	return n;
}
</xsl:if>


bool <xsl:value-of select="$controlClass"/>::<xsl:apply-templates select="." mode="method"/>
{
	<!-- Build request and send it, using a lambda wrapper for response handling -->
	Envelope request(*this);
	<xsl:variable name="action"><xsl:apply-templates select="." mode="name"/></xsl:variable>
	request.createRequest(Action::<xsl:value-of select="$action"/>);<xsl:text/>
	<xsl:for-each select="s:argumentList/s:argument[s:direction='in']">
	request.addArg(<xsl:value-of select="$action"/>::Arg::<xsl:call-template name="varname"/>, <xsl:call-template name="varname-cpp"/>);<xsl:text/>
	</xsl:for-each>
	return sendRequest(request, [callback](UPnP::Envelope&amp; response) {
		<xsl:value-of select="$action"/>::Result result(response);
		callback(result);
	});
}
</xsl:for-each>

<xsl:call-template name="namespace-close"/>

</xsl:template>



</xsl:stylesheet>
