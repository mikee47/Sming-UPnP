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
<xsl:call-template name="file-hpp"/>
<xsl:call-template name="namespace-open"/>

class <xsl:call-template name="control-class"/>: public ServiceControl
{
public:
	class Class: public ServiceClass
	{
	public:
		using ServiceClass::ServiceClass;
	
		String getField(Field desc) const override;
		Version version() const override;

	protected:
		ServiceControl* createObject(DeviceControl&amp; device, const ServiceClass&amp; serviceClass) const override
		{
			return new <xsl:call-template name="control-class"/>(device, serviceClass);
		}
	}; // Class

	using ServiceControl::ServiceControl;

	<xsl:apply-templates select="s:actionList/s:action"/>
};

<xsl:call-template name="namespace-close"/>

// Alias for easier use
using <xsl:call-template name="control-class"/> = UPnP::service::<xsl:call-template name="urn-domain-cpp"/>::<xsl:call-template name="control-class"/>;

</xsl:template>

<xsl:template match="s:action">
	<!-- Declare a struct to contain result arguments, with an appropriate callback delegate type -->
	/**
	 * @brief Action: <xsl:call-template name="action-name"/>
	 * @{
	 */<xsl:text/>
	 
	<xsl:choose>
	<xsl:when test="s:argumentList/s:argument[s:direction='out']">
	struct <xsl:call-template name="action-result"/> {
		<xsl:for-each select="s:argumentList/s:argument[s:direction='out']">
		<xsl:call-template name="variable-type"/><xsl:text> </xsl:text><xsl:call-template name="varname-cpp"/>;<xsl:text/>
		</xsl:for-each>

		size_t printTo(Print&amp; p);
	};
	using <xsl:call-template name="action-result-callback"/> = Delegate&lt;void(<xsl:call-template name="action-result"/>&amp; result)>;<xsl:text/>
	</xsl:when>
	<xsl:otherwise>
	using <xsl:call-template name="action-result-callback"/> = Delegate&lt;void()>;<xsl:text/>
	</xsl:otherwise>
	</xsl:choose>

	bool <xsl:call-template name="action-method"/>;
	/** @} */
</xsl:template>


</xsl:stylesheet>
