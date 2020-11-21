<?xml version='1.0'?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:str="http://exslt.org/strings"
	xmlns:d="urn:schemas-upnp-org:device-1-0"
	xmlns:s="urn:schemas-upnp-org:service-1-0">


<!-- Used to camelCase things (libxml2 implements xslt 1.0) -->
<xsl:variable name="vLower" select="'abcdefghijklmnopqrstuvwxyz'"/>
<xsl:variable name="vUpper" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'"/>
 
 <!-- Header file opening -->
<xsl:template name="file-hpp">
/**
 * @brief <xsl:value-of select="d:deviceType | s:serviceType"/>
 *
 * @note This file is auto-generated ** DO NOT EDIT **
 *
 * To customise, create a new class and inherit from `<xsl:call-template name="control-class"/>`.
 *
 * If any new member variables or virtual methods are introduced, you will also need to
 * overide `<xsl:call-template name="control-class"/>::Class`.
 *  
 */

#pragma once

#include &lt;Network/UPnP/DeviceControl.h>
#include &lt;Network/UPnP/ServiceControl.h>
</xsl:template>

<!-- Source file comment -->
<xsl:template name="file-cpp">
/**
 * @brief <xsl:value-of select="d:deviceType | s:serviceType"/>
 *
 * @note This file is auto-generated ** DO NOT EDIT **
 *
 */

#include "<xsl:call-template name="control-class"/>.h"

</xsl:template>

<!-- Namespace opening declaration -->
<xsl:template name="namespace-open">
namespace UPnP {
namespace <xsl:call-template name="urn-domain-cpp"/> {
namespace <xsl:call-template name="urn-kind"/> {
</xsl:template>

<!-- Namespace opening declaration -->
<xsl:template name="namespace-close">
} // namespace <xsl:call-template name="urn-kind"/>
} // namespace <xsl:call-template name="urn-domain-cpp"/>
} // namespace UPnP
</xsl:template>

<!-- Breaks device/service URN into a valid C++ directory path -->
<xsl:template name="header-path">
<xsl:variable name="elem" select="str:tokenize(d:deviceType | d:serviceType | s:serviceType, ':')"/>
<xsl:value-of select="concat($elem[2], '/', $elem[3], '/', $elem[4], $elem[5], '.h')"/>
</xsl:template>

<!-- Device or service type with version, e.g. RenderingControl1 -->
<xsl:template name="control-class">
<xsl:variable name="elem" select="str:tokenize(d:deviceType | d:serviceType | s:serviceType, ':')"/>
<xsl:value-of select="concat($elem[4], $elem[5])"/>
</xsl:template>

<!-- Just the device or service type, no version, e.g. RenderingControl -->
<xsl:template name="control-name"><xsl:call-template name="urn-type"/></xsl:template>

<!-- Domain element of URN -->
<xsl:template name="urn-domain"><xsl:value-of select="str:tokenize(d:deviceType | d:serviceType | s:serviceType, ':')[2]"/></xsl:template>

<!-- Domain element of URN in C++ compatible format -->
<xsl:template name="urn-domain-cpp">
<xsl:variable name="elem" select="str:tokenize(d:deviceType | d:serviceType | s:serviceType, ':')"/>
<xsl:value-of select="translate($elem[2], '.-', '__')"/>
</xsl:template>

<!-- Will be "device" or "service" -->
<xsl:template name="urn-kind"><xsl:value-of select="str:tokenize(d:deviceType | d:serviceType | s:serviceType, ':')[3]"/></xsl:template>

<!-- Type element of URN -->
<xsl:template name="urn-type"><xsl:value-of select="str:tokenize(d:deviceType | d:serviceType | s:serviceType, ':')[4]"/></xsl:template>

<!-- Version element of URN -->
<xsl:template name="urn-version"><xsl:value-of select="str:tokenize(d:deviceType | d:serviceType | s:serviceType, ':')[5]"/></xsl:template>

<!-- State variable type -->
<xsl:template match="s:stateVariable" mode="type">
	<xsl:param name="const"/>
	<xsl:call-template name="cpp-vartype">
		<xsl:with-param name="vartype" select="s:dataType"/>
		<xsl:with-param name="const" select="$const"/>
	</xsl:call-template>
</xsl:template>

<!-- State variable name -->
<xsl:template match="s:stateVariable" mode="name"><xsl:value-of select="str:replace(s:name, 'A_ARG_TYPE_', '')"/></xsl:template>

<!-- State variable allowed value name (tag) -->
<xsl:template match="s:allowedValue" mode="name">fs_<xsl:value-of select="translate(., '-./', '___')"/></xsl:template>

<!-- Action argument type -->
<xsl:template match="s:argument" mode="type">
	<xsl:param name="const"/>
	<xsl:apply-templates select="/s:scpd/s:serviceStateTable/s:stateVariable[s:name=current()/s:relatedStateVariable]" mode="type">
		<xsl:with-param name="const" select="$const"/>
	</xsl:apply-templates>
</xsl:template>

<!-- Name of action with weird prefixes removed -->
<xsl:template match="s:action" mode="name">
<xsl:value-of select="str:replace(s:name, 'X_', '')"/>
</xsl:template>

<!-- Name of variable/parameter with weird prefixes removed, for display -->
<xsl:template name="varname"><xsl:value-of select="str:replace(s:name, 'X_', '')"/></xsl:template>

<!-- Name of variable/parameter, for C++ use -->
<xsl:template name="varname-cpp">v<xsl:call-template name="varname"/></xsl:template>

<!-- Name of structure containing action result -->
<xsl:template name="action-result"><xsl:apply-templates select="." mode="name"/>Result</xsl:template>

<!-- Name of result Delegate type -->
<xsl:template name="action-result-callback"><xsl:call-template name="action-result"/>Callback</xsl:template>

<!-- Method declaration for service action -->
<xsl:template match="s:action" mode="method">
	<xsl:variable name="name"><xsl:apply-templates select="." mode="name"/></xsl:variable>
	<xsl:value-of select="concat(translate(substring($name,1,1), $vUpper, $vLower), substring($name, 2))"/>(
		<xsl:for-each select="s:argumentList/s:argument[s:direction='in']">
		<xsl:apply-templates select="." mode="type">
			<xsl:with-param name="const" select="1"/>
		</xsl:apply-templates>
		<xsl:text> </xsl:text><xsl:call-template name="varname-cpp"/>,
		</xsl:for-each>
		<xsl:call-template name="action-result-callback"/> callback)<xsl:text/>
</xsl:template>

<!-- Map an action variable type onto the appropriate C++ one -->
<xsl:template name="cpp-vartype">
	<xsl:param name="vartype"/>
	<xsl:param name="const"/>
	<xsl:choose>
	    <xsl:when test="$vartype='string' and $const">const String&amp;</xsl:when>
	    <xsl:when test="$vartype='string' and not($const)">String</xsl:when>
	    <xsl:when test="$vartype='ui1'">uint8_t</xsl:when>
	    <xsl:when test="$vartype='ui2'">uint16_t</xsl:when>
	    <xsl:when test="$vartype='ui4'">uint32_t</xsl:when>
	    <xsl:when test="$vartype='i1'">int8_t</xsl:when>
	    <xsl:when test="$vartype='i2'">int16_t</xsl:when>
	    <xsl:when test="$vartype='i4'">int32_t</xsl:when>
	    <xsl:when test="$vartype='int'">int</xsl:when>
	    <xsl:when test="$vartype='boolean'">bool</xsl:when>
	    <xsl:when test="$vartype='r4'">float</xsl:when>
	    <xsl:when test="$vartype='r8'">double</xsl:when>
	    <xsl:when test="$vartype='number'">double</xsl:when>
	    <xsl:when test="$vartype='float'">float</xsl:when>
	    <xsl:when test="$vartype='char'">char</xsl:when>
 	    <xsl:when test="$vartype='bin.base64' and $const">const Base64&amp;</xsl:when>
 	    <xsl:when test="$vartype='bin.base64' and not($const)">Base64</xsl:when>
<!--
fixed.14.4
date
dateTime
dateTime.tz
time
time.tz
bin.hex
uri
uuid
 -->
		<xsl:otherwise>
      		<xsl:message terminate="yes">Unknown variable type: "<xsl:value-of select="$vartype"/>"</xsl:message>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>


</xsl:stylesheet>
