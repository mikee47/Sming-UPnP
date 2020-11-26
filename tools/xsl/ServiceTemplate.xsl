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
template &lt;class S> class <xsl:value-of select="$templateClass"/>: public Service
{
public:
	using Service::Service;
	<xsl:for-each select="s:actionList/s:action">
	<xsl:variable name="name"><xsl:apply-templates select="." mode="name"/></xsl:variable>
	using <xsl:value-of select="concat($name, ' = ', $controlClass, '::', $name)"/>;<xsl:text/>
	</xsl:for-each>

	const ObjectClass&amp; getClass() const override
	{
		return <xsl:value-of select="$controlClass"/>_class;
	}

	void handleAction(Envelope&amp; env) override
	{
		String actionName = env.actionName();
		ActionRequest req(env);
		<xsl:for-each select="s:actionList/s:action">
		<xsl:variable name="name"><xsl:apply-templates select="." mode="name"/></xsl:variable>
		<xsl:variable name="Arg" select="concat($name, '::Arg::')"/>
		if(<xsl:value-of select="concat($name, '::actionName')"/> == actionName) {
			static_cast&lt;S*>(this)-><xsl:value-of select="concat(translate(substring($name,1,1), $vUpper, $vLower), substring($name, 2))"/>(<xsl:text/>
			<xsl:for-each select="s:argumentList/s:argument[s:direction='in']">
				req.getArg&lt;<xsl:apply-templates select="." mode="type"/>>(<xsl:text/>
				<xsl:value-of select="$Arg"/><xsl:call-template name="varname"/>),<xsl:text/>
				</xsl:for-each>
				<xsl:value-of select="concat($name, '::Result')"/>(env.createResponse(actionName)));
			return;
		}
		</xsl:for-each>
	}
};
<xsl:call-template name="namespace-close"/>
</xsl:template>

</xsl:stylesheet>
