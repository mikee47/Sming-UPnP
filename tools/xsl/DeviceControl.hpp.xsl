<?xml version='1.0'?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:str="http://exslt.org/strings"
	xmlns:d="urn:schemas-upnp-org:device-1-0">
<xsl:import href="common.xsl"/>
<xsl:output method="text" />

<xsl:template match="d:device">
<xsl:variable name="controlClass"><xsl:call-template name="control-class"/></xsl:variable>
<xsl:call-template name="file-control-hpp"/>
<xsl:for-each select="d:serviceList/d:service">
<xsl:text/>#include &lt;Network/UPnP/<xsl:call-template name="file-path"/>.h&gt;
</xsl:for-each>

<xsl:call-template name="namespace-open"/>
extern const ObjectClass <xsl:value-of select="$controlClass"/>_class;

class <xsl:value-of select="$controlClass"/>: public DeviceControl
{
public:
	using DeviceControl::DeviceControl;
	
	const ObjectClass&amp; getClass() const override
	{
		return <xsl:value-of select="$controlClass"/>_class;
	}

	static Object* createObject(DeviceControl* owner)
	{
		return new <xsl:value-of select="$controlClass"/>(owner);
	}

	<xsl:for-each select="d:serviceList/d:service">
	<xsl:text>
	</xsl:text>
	<xsl:variable name="class"><xsl:call-template name="control-class-full"/></xsl:variable>
	<xsl:value-of select="$class"/>* get<xsl:call-template name="control-name"/>()
	{
		auto service = getService(<xsl:value-of select="$class"/>_class.objectType());
		return reinterpret_cast&lt;<xsl:value-of select="$class"/>*>(service);
	}
	</xsl:for-each>

	<xsl:for-each select="d:deviceList/d:device">
	<xsl:text>
	</xsl:text>
	<xsl:variable name="class"><xsl:call-template name="control-class-full"/></xsl:variable>
	<xsl:value-of select="$class"/>* get<xsl:call-template name="control-name"/>()
	{
		auto device = getDevice(<xsl:value-of select="$class"/>_class.objectType());
		return reinterpret_cast&lt;<xsl:value-of select="$class"/>*>(device);
	}
	</xsl:for-each>
};
<xsl:call-template name="namespace-close"/>
</xsl:template>

</xsl:stylesheet>
