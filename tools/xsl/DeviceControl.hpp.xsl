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

class <xsl:call-template name="control-class"/>: public UPnP::DeviceControl
{
public:
	class Class: public UPnP::DeviceClass
	{
	public:
		Class():
			<xsl:for-each select="d:serviceList/d:service">
			<xsl:if test="position() > 1">,
			</xsl:if><xsl:call-template name="control-name"/>(*this)</xsl:for-each>
		{<xsl:text/>
			<xsl:for-each select="d:serviceList/d:service">
			serviceClasses.add(&amp;<xsl:call-template name="control-name"/>);<xsl:text/>
			</xsl:for-each>
		}

		/**
		 * @brief Core field definitions
		 */
		String getField(Field desc) const override;
		Version version() const override;

		<xsl:if test="d:URLBase">
		<xsl:message terminate="yes">URLBase NOT PERMITTED</xsl:message>
		</xsl:if>

		<xsl:for-each select="d:serviceList/d:service">
		const <xsl:call-template name="control-class"/>::Class <xsl:call-template name="control-name"/>;<xsl:text/>
		</xsl:for-each>

	protected:
		UPnP::DeviceControl* createObject(UPnP::ControlPoint&amp; controlPoint) const override
		{
			return new <xsl:call-template name="control-class"/>(*this, controlPoint);
		}
	}; // Class

	using DeviceControl::DeviceControl;
	
	const Class&amp; getClass() const
	{
		return reinterpret_cast&lt;const Class&amp;>(DeviceControl::getClass());
	}

	<xsl:for-each select="d:serviceList/d:service">
	<xsl:text>
	</xsl:text>
	<xsl:call-template name="control-class"/>&amp; get<xsl:call-template name="control-name"/>()
	{
		auto service = getService(getClass().<xsl:call-template name="control-name"/>);
		return *reinterpret_cast&lt;<xsl:call-template name="control-class"/>*>(service);
	}
	</xsl:for-each>
};

<xsl:call-template name="namespace-close"/>

</xsl:template>

</xsl:stylesheet>
