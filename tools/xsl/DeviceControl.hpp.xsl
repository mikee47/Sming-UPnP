<?xml version='1.0'?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:str="http://exslt.org/strings"
	xmlns:d="urn:schemas-upnp-org:device-1-0">
<xsl:import href="common.xsl"/>
<xsl:output method="text" />

<xsl:template match="d:device">
<xsl:call-template name="file-hpp"/>

<xsl:for-each select="d:serviceList/d:service">
#include &lt;Network/UPnP/<xsl:call-template name="header-path" select="d:serviceType"/>&gt;
</xsl:for-each>

<xsl:call-template name="namespace-open"/>

<xsl:variable name="controlClass"><xsl:call-template name="control-class"/></xsl:variable>

class <xsl:value-of select="$controlClass"/>: public UPnP::DeviceControl
{
public:
	using DeviceControl::DeviceControl;
	
	static const DeviceClass class_;

	const DeviceClass&amp; getClass() const override
	{
		return class_;
	}

	static Object* createObject(DeviceControl* owner)
	{
		return new <xsl:value-of select="$controlClass"/>(owner);
	}

	<xsl:for-each select="d:serviceList/d:service">
	<xsl:text>
	</xsl:text>
	<xsl:call-template name="control-class"/>* get<xsl:call-template name="control-name"/>()
	{
		return getService&lt;<xsl:call-template name="control-class"/>>();
	}
	</xsl:for-each>
};

<xsl:call-template name="namespace-close"/>

// Alias for easier use
using <xsl:call-template name="control-class"/> = UPnP::<xsl:call-template name="urn-domain-cpp"/>::device::<xsl:call-template name="control-class"/>;

</xsl:template>

</xsl:stylesheet>
