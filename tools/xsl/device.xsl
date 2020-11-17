<?xml version='1.0'?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:u="urn:schemas-upnp-org:device-1-0"
	xmlns:s="urn:schemas-upnp-org:service-1-0"
	xmlns:fn="http://www.w3.org/2005/xpath-functions">
<xsl:output method="text" />

<xsl:param name="DOC_ROOT"/>

<!-- 
<xsl:variable name="DOC_ROOT" select="'../../samples/Basic_ControlPoint/config'"/>
 -->

<xsl:template match="u:root">
<xsl:apply-templates select="u:device"/>
</xsl:template>

<xsl:template match="u:device">
/**
 * @brief ControlPoint device description for <xsl:value-of select="u:deviceType"/>
 * @note This file is auto-generated ** DO NOT EDIT **
 */

#pragma once

#include &lt;Network/UPnP/DeviceControl.h>
#include &lt;Network/UPnP/ServiceControl.h>

<xsl:variable name="type" select="substring-before(substring-after(u:deviceType, ':device:'), ':')"/>
<xsl:variable name="deviceControl">Device_<xsl:value-of select="$type"/></xsl:variable>

<xsl:apply-templates select="u:serviceList/u:service"/>

class <xsl:value-of select="$deviceControl"/>: public UPnP::DeviceControl
{
public:
	class Class: public UPnP::DeviceClass
	{
	public:
		Class():
			<xsl:for-each select="u:serviceList/u:service">
			<xsl:if test="position() > 1">,
			</xsl:if><xsl:call-template name="serviceType"/>(*this)</xsl:for-each>
		{<xsl:text/>
			<xsl:for-each select="u:serviceList/u:service">
			serviceClasses.add(&amp;<xsl:call-template name="serviceType"/>);<xsl:text/>
			</xsl:for-each>
		}

		/**
		 * @brief Core field definitions
		 */
		String getField(Field desc) const override
		{
			switch(desc) {
			case Field::domain:
				return domain;
			case Field::friendlyName:
				return friendlyName;
			case Field::type:
				return type;
			case Field::version:
				return String(version_);
			case Field::manufacturer:
				return manufacturer;
			case Field::modelName:
				return modelName;
			case Field::modelNumber:
				return modelNumber;
			default:
				return DeviceClass::getField(desc);
			}
		}
		
		Version version() const override
		{
			return version_;
		} 
		
		<xsl:if test="u:URLBase">
		<xsl:message terminate="yes">URLBase NOT PERMITTED</xsl:message>
		</xsl:if>

		/*
		 * Interface specification
		 */
		DEFINE_FSTR_LOCAL(domain, "<xsl:value-of select="substring-before(substring-after(u:deviceType, ':'), ':')"/>");
		DEFINE_FSTR_LOCAL(friendlyName,"<xsl:value-of select="u:friendlyName"/>");
		DEFINE_FSTR_LOCAL(type, "<xsl:value-of select="$type"/>");
		static constexpr uint8_t version_ = <xsl:value-of select="substring-after(substring-after(u:deviceType, ':device:'), ':')"/>;

		/*
		 * Common properties
		 */
		DEFINE_FSTR_LOCAL(manufacturer, "<xsl:value-of select="u:manufacturer"/>");
		DEFINE_FSTR_LOCAL(modelName, "<xsl:value-of select="u:modelName"/>");
		DEFINE_FSTR_LOCAL(modelNumber, "<xsl:value-of select="u:modelNumber"/>");

		<xsl:for-each select="u:serviceList/u:service">
		const <xsl:call-template name="serviceControl"/>::Class <xsl:call-template name="serviceType"/>;<xsl:text/>
		</xsl:for-each>

	protected:
		UPnP::DeviceControl* createObject(UPnP::ControlPoint&amp; controlPoint) const override
		{
			return new <xsl:value-of select="$deviceControl"/>(*this, controlPoint);
		}
	}; // Class

	using DeviceControl::DeviceControl;
	
	const Class&amp; getClass() const
	{
		return reinterpret_cast&lt;const Class&amp;>(DeviceControl::getClass());
	}

	<xsl:for-each select="u:serviceList/u:service">
	<xsl:text>
	</xsl:text>
	<xsl:call-template name="serviceControl"/>&amp; get<xsl:call-template name="serviceType"/>()
	{
		auto service = getService(getClass().<xsl:call-template name="serviceType"/>);
		return *reinterpret_cast&lt;<xsl:call-template name="serviceControl"/>*>(service);
	}
	</xsl:for-each>
};

</xsl:template>


<xsl:template name="serviceType"><xsl:value-of select="substring-before(substring-after(u:serviceType, ':service:'), ':')"/></xsl:template>
<xsl:template name="serviceControl">Service_<xsl:call-template name="serviceType"/></xsl:template>

<xsl:template match="u:service">
<xsl:variable name="SCPDURL" select="concat($DOC_ROOT,u:SCPDURL)"/>
<xsl:variable name="type"><xsl:call-template name="serviceType"/></xsl:variable>
<xsl:variable name="serviceControl"><xsl:call-template name="serviceControl"/></xsl:variable>

class <xsl:value-of select="$serviceControl"/>: public UPnP::ServiceControl
{
public:
	class Class: public UPnP::ServiceClass
	{
	public:
		using ServiceClass::ServiceClass;
	
		String getField(Field desc) const override
		{
			switch(desc) {
			case Field::domain:
				return domain;
			case Field::type:
				return type;
			case Field::version:
				return String(version_);
			case Field::serviceId:
				return serviceId;
			case Field::SCPDURL:
				return SCPDURL;
			case Field::controlURL:
				return controlURL;
			case Field::eventSubURL:
				return eventSubURL;
			default:
				return ServiceClass::getField(desc);
			}
		}

		Version version() const override
		{
			return version_;
		} 

		/*
		 * Interface specification
		 */
		DEFINE_FSTR_LOCAL(domain, "<xsl:value-of select="substring-before(substring-after(u:serviceType, ':'), ':')"/>");
		DEFINE_FSTR_LOCAL(type, "<xsl:value-of select="$type"/>");
		static constexpr uint8_t version_ = <xsl:value-of select="substring-after(substring-after(u:serviceType, ':service:'), ':')"/>;

		/*
		 * Common properties
		 */
		DEFINE_FSTR_LOCAL(serviceId, "<xsl:value-of select="u:serviceId"/>");
		DEFINE_FSTR_LOCAL(SCPDURL, "<xsl:value-of select="u:SCPDURL"/>");
		DEFINE_FSTR_LOCAL(controlURL, "<xsl:value-of select="u:controlURL"/>");
		DEFINE_FSTR_LOCAL(eventSubURL, "<xsl:value-of select="u:eventSubURL"/>");

	protected:
		UPnP::ServiceControl* createObject(UPnP::DeviceControl&amp; device, const ServiceClass&amp; serviceClass) const override
		{
			return new <xsl:value-of select="$serviceControl"/>(device, serviceClass);
		}
	}; // Class
	
 	<xsl:apply-templates select="document($SCPDURL)/s:scpd/s:serviceStateTable/s:stateVariable"/>

	using ServiceControl::ServiceControl;

	<xsl:apply-templates select="document($SCPDURL)/s:scpd/s:actionList/s:action"/>
};

</xsl:template>

<xsl:template match="s:action">
	<!-- Declare a struct to contain result arguments, with an appropriate callback delegate type -->
	<xsl:variable name="ResultArgs">Result_<xsl:value-of select="s:name"/></xsl:variable>
	<xsl:choose>
	<xsl:when test="s:argumentList/s:argument[s:direction='out']">
	struct <xsl:value-of select="$ResultArgs"/> {
		<xsl:for-each select="s:argumentList/s:argument[s:direction='out']">
		<xsl:value-of select="s:relatedStateVariable"/><xsl:text> </xsl:text><xsl:value-of select="s:name"/>;
		</xsl:for-each>
		
		size_t printTo(Print&amp; p) {
			size_t n{0};
			<xsl:for-each select="s:argumentList/s:argument[s:direction='out']">
			n += p.print("<xsl:value-of select="s:name"/>");
			n += p.print(" = ");
			n += p.println(<xsl:value-of select="s:name"/>);
			</xsl:for-each>
			return n;
		}
	};
	using <xsl:value-of select="s:name"/>_ResultCallback = Delegate&lt;void(<xsl:value-of select="$ResultArgs"/>&amp; result)>;
	</xsl:when>
	<xsl:otherwise>
	using <xsl:value-of select="s:name"/>_ResultCallback = Delegate&lt;void()>;
	</xsl:otherwise>
	</xsl:choose>

	<!-- Action execution -->
	bool action_<xsl:value-of select="s:name"/>(
			<xsl:for-each select="s:argumentList/s:argument[s:direction='in']">
			<xsl:value-of select="s:relatedStateVariable"/><xsl:text> </xsl:text><xsl:value-of select="s:name"/>,
			</xsl:for-each>
			<xsl:value-of select="s:name"/>_ResultCallback callback)
	{
		<!-- Build request and send it, using a lambda wrapper for response handling -->
		UPnP::ActionInfo request(*this);
		request.createRequest(F("<xsl:value-of select="s:name"/>"));<xsl:text/>
		<xsl:for-each select="s:argumentList/s:argument[s:direction='in']">
		request.addArg(F("<xsl:value-of select="s:name"/>"), <xsl:value-of select="s:name"/>);<xsl:text/>
		</xsl:for-each>
		return sendRequest(request, [callback](UPnP::ActionInfo&amp; response) {
			<!-- Process response and invoke user callback -->
			<xsl:choose>
			<xsl:when test="s:argumentList/s:argument[s:direction='out']">
			<xsl:value-of select="$ResultArgs"/> result;
			<xsl:for-each select="s:argumentList/s:argument[s:direction='out']">
			response.getArg(F("<xsl:value-of select="s:name"/>"), result.<xsl:value-of select="s:name"/>);<xsl:text/>
			</xsl:for-each>
			callback(result);
			</xsl:when>
			<xsl:otherwise>
			callback();
			</xsl:otherwise>
			</xsl:choose>
		});
	}
</xsl:template>


<xsl:template match="s:stateVariable">
	using <xsl:value-of select="s:name"/> = <xsl:call-template name="getvartype"/>;<xsl:text/>
</xsl:template>


<xsl:template name="getvartype">
  <xsl:choose>
    <xsl:when test="s:dataType='string'">String</xsl:when>
    <xsl:when test="s:dataType='ui1'">uint8_t</xsl:when>
    <xsl:when test="s:dataType='ui2'">uint16_t</xsl:when>
    <xsl:when test="s:dataType='ui4'">uint32_t</xsl:when>
    <xsl:when test="s:dataType='i1'">int8_t</xsl:when>
    <xsl:when test="s:dataType='i2'">int16_t</xsl:when>
    <xsl:when test="s:dataType='i4'">int32_t</xsl:when>
    <xsl:when test="s:dataType='int'">int</xsl:when>
    <xsl:when test="s:dataType='boolean'">bool</xsl:when>
    <xsl:when test="s:dataType='r4'">float</xsl:when>
    <xsl:when test="s:dataType='r8'">double</xsl:when>
    <xsl:when test="s:dataType='number'">double</xsl:when>
    <xsl:when test="s:dataType='float'">float</xsl:when>
    <xsl:when test="s:dataType='char'">char</xsl:when>
<!--
fixed.14.4
date
dateTime
dateTime.tz
time
time.tz
bin.base64
bin.hex
uri
uuid
 -->
     <xsl:otherwise>
      <xsl:message terminate="yes">Unknown field type: "<xsl:value-of select="s:dataType"/>"</xsl:message>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template name="urn-domain">
	<xsl:value-of select="substring-after(., ':')"/>
</xsl:template>

<xsl:template name="urn-type">
	<xsl:value-of select="substring-after(., ':')"/>
</xsl:template>

</xsl:stylesheet>
