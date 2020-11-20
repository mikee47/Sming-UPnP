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
DEFINE_FSTR_LOCAL(FS_type, "<xsl:call-template name="urn-type"/>");
static constexpr uint8_t versionNumber{<xsl:call-template name="urn-version"/>};

} // namespace

String <xsl:call-template name="control-class"/>::Class::getField(Field desc) const
{
	switch(desc) {
	case Field::domain:
		return FS_domain;
	case Field::type:
		return FS_type;
	case Field::version:
		return String(versionNumber);
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
