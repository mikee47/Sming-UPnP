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
<xsl:call-template name="namespace-open"/>

namespace {
/*
 * Interface specification
 */
DEFINE_FSTR_LOCAL(FS_domain, "<xsl:call-template name="urn-domain"/>");
DEFINE_FSTR_LOCAL(FS_friendlyName,"<xsl:value-of select="d:friendlyName"/>");
DEFINE_FSTR_LOCAL(FS_type, "<xsl:call-template name="urn-type"/>");
static constexpr uint8_t versionNumber{<xsl:call-template name="urn-version"/>};

/*
 * Common properties
 */
DEFINE_FSTR_LOCAL(FS_manufacturer, "<xsl:value-of select="d:manufacturer"/>");
DEFINE_FSTR_LOCAL(FS_modelName, "<xsl:value-of select="d:modelName"/>");
DEFINE_FSTR_LOCAL(FS_modelNumber, "<xsl:value-of select="d:modelNumber"/>");

} // namespace

String <xsl:call-template name="control-class"/>::Class::getField(Field desc) const
{
	switch(desc) {
	case Field::domain:
		return FS_domain;
	case Field::friendlyName:
		return FS_friendlyName;
	case Field::type:
		return FS_type;
	case Field::version:
		return String(versionNumber);
	case Field::manufacturer:
		return FS_manufacturer;
	case Field::modelName:
		return FS_modelName;
	case Field::modelNumber:
		return FS_modelNumber;
	default:
		return DeviceClass::getField(desc);
	}
}
		
Object::Version <xsl:call-template name="control-class"/>::Class::version() const
{
	return versionNumber;
}

<xsl:call-template name="namespace-close"/>

</xsl:template>

</xsl:stylesheet>
