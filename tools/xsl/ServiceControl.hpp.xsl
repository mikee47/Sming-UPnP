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

<xsl:variable name="controlClass"><xsl:call-template name="control-class"/></xsl:variable>

class <xsl:value-of select="$controlClass"/>: public ServiceControl
{
public:
	/*
	 * Pre-defined values (from allowed value lists)
	 */
	<xsl:for-each select="s:serviceStateTable/s:stateVariable[s:allowedValueList]">
	struct <xsl:apply-templates select="." mode="name"/> {<xsl:text/>
	<xsl:for-each select="s:allowedValueList/s:allowedValue">
		DEFINE_FSTR_LOCAL(<xsl:apply-templates select="." mode="name"/>, "<xsl:value-of select="."/>")<xsl:text/>
	</xsl:for-each>
	};
	</xsl:for-each>

	using ServiceControl::ServiceControl;

	static const ServiceClass class_;

	const ServiceClass&amp; getClass() const override
	{
		return class_;
	}

	static Object* createObject(DeviceControl* owner)
	{
		return owner ? new <xsl:value-of select="$controlClass"/>(*owner) : nullptr;
	}

	<xsl:apply-templates select="s:actionList/s:action"/>
};

<xsl:call-template name="namespace-close"/>

// Alias for easier use
using <xsl:value-of select="$controlClass"/> = UPnP::<xsl:call-template name="urn-domain-cpp"/>::service::<xsl:value-of select="$controlClass"/>;

</xsl:template>

<xsl:template match="s:action">
	<!-- Declare a struct to contain result arguments, with an appropriate callback delegate type -->
	/**
	 * @brief Action: <xsl:value-of select="s:name"/>
	 * @{
	 */<xsl:text/>

	<xsl:choose>
	<xsl:when test="s:argumentList/s:argument[s:direction='out']">
	struct <xsl:call-template name="action-result"/> {
		<xsl:for-each select="s:argumentList/s:argument[s:direction='out']">
		<xsl:apply-templates select="." mode="type"/><xsl:text> </xsl:text><xsl:call-template name="varname-cpp"/>;
		</xsl:for-each>
		size_t printTo(Print&amp; p);
	};
	using <xsl:call-template name="action-result-callback"/> = Delegate&lt;void(<xsl:call-template name="action-result"/>&amp; result)>;<xsl:text/>
	</xsl:when>
	<xsl:otherwise>
	using <xsl:call-template name="action-result-callback"/> = Delegate&lt;void()>;<xsl:text/>
	</xsl:otherwise>
	</xsl:choose>

	bool <xsl:apply-templates select="." mode="method"/>;
	/** @} */
</xsl:template>


</xsl:stylesheet>
