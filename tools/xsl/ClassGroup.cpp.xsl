<?xml version='1.0'?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:str="http://exslt.org/strings"
	xmlns:d="urn:schemas-upnp-org:device-1-0"
	xmlns:s="urn:schemas-upnp-org:service-1-0">
<xsl:import href="common.xsl"/>
<xsl:output method="text" />

<xsl:template match="root">

<xsl:variable name="domain">
<xsl:for-each select="(s:scpd | d:device)[position()=1]"><xsl:call-template name="urn-domain"/></xsl:for-each>
</xsl:variable>

<xsl:variable name="domain-cpp">
<xsl:for-each select="(s:scpd | d:device)[position()=1]"><xsl:call-template name="urn-domain-cpp"/></xsl:for-each>
</xsl:variable>

/**
 * @brief Class information for "<xsl:value-of select="$domain"/>"
 *
 * @note This file is auto-generated ** DO NOT EDIT **
 */

#include "ClassGroup.h"
#include &lt;Network/UPnP/ControlPoint.h>

namespace UPnP {
namespace <xsl:value-of select="$domain-cpp"/> {

DEFINE_FSTR_LOCAL(domain_, "<xsl:value-of select="$domain"/>");

const FlashString&amp; domain()
{
	return domain_;
}


<xsl:for-each select="s:scpd | d:device">
namespace <xsl:call-template name="urn-kind"/> {
DEFINE_FSTR_LOCAL(<xsl:call-template name="control-class"/>_type, "<xsl:call-template name="urn-type"/>");
IMPORT_FSTR_LOCAL(<xsl:call-template name="control-class"/>_schema, COMPONENT_PATH "/schema/<xsl:call-template name="file-path"/>.xml")

constexpr ObjectClass <xsl:call-template name="control-class"/>_class PROGMEM = {
	&amp;domain_,
	&amp;<xsl:call-template name="control-class"/>_type,
	&amp;<xsl:call-template name="control-class"/>_schema,
	<xsl:call-template name="urn-version"/>,
	Urn::Kind::<xsl:call-template name="urn-kind"/>,
	<xsl:call-template name="control-class"/>::createObject
};

static_assert(std::is_pod&lt;decltype(<xsl:call-template name="control-class"/>_class)>::value, "ObjectClass structure not POD");                                       \
} // namespace <xsl:call-template name="urn-kind"/>


</xsl:for-each>)

DEFINE_FSTR_VECTOR_LOCAL(classes, ObjectClass,
	<xsl:for-each select="s:scpd | d:device">&amp;<xsl:call-template name="urn-kind"/>::<xsl:call-template name="control-class"/>_class,
	</xsl:for-each>)

void registerClasses()
{
	ControlPoint::registerClasses(domain_, classes);
}

} // namespace <xsl:value-of select="$domain-cpp"/>
} // namespace UPnP

</xsl:template>

</xsl:stylesheet>
