/**
 * Envelope.cpp
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the Sming UPnP Library
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/Network/UPnP/Envelope.h"
#include "include/Network/UPnP/Service.h"

namespace UPnP
{
namespace
{
DEFINE_FSTR_LOCAL(soap_namespace, "http://schemas.xmlsoap.org/soap/envelope/");
DEFINE_FSTR_LOCAL(soap_encoding, "http://schemas.xmlsoap.org/soap/encoding/");
DEFINE_FSTR_LOCAL(xmlns_u, "xmlns:u")
DEFINE_FSTR_LOCAL(xmlns_s, "xmlns:s")
DEFINE_FSTR_LOCAL(s_Envelope, "s:Envelope")
DEFINE_FSTR_LOCAL(s_encodingStyle, "s:encodingStyle")
DEFINE_FSTR_LOCAL(s_Body, "s:Body")
DEFINE_FSTR_LOCAL(s_Client, "s:Client")
DEFINE_FSTR_LOCAL(s_Fault, "s:Fault")

#define LOCALSTR(x) DEFINE_FSTR_LOCAL(fs_##x, #x)

LOCALSTR(Response)
LOCALSTR(Fault)
LOCALSTR(UPnPError)
LOCALSTR(faultcode)
LOCALSTR(faultstring)
LOCALSTR(detail)
LOCALSTR(errorCode)
LOCALSTR(errorDescription)
} // namespace

Error Envelope::parseBody()
{
	if(!XML::deserialize(doc, buffer)) {
		debug_e("[UPnP] Error deserializing XML");
		return Error::XmlParsing;
	}

	auto body = XML::getNode(doc, F("Envelope/Body"), soap_namespace);

	if(body == nullptr) {
		debug_e("[SOAP] Body missing");
		return Error::NoSoapBody;
	}

	/*
	 * e.g.
	 *
	 * <u:Browse xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1">
	 *
	 * or
	 *
	 * <s:Fault>
	 *
	 */
	content = body->first_node();
	if(content == nullptr) {
		debug_e("[UPnP] Envelope is empty");
		return Error::NoSoapContent;
	}

	name.setString(content->name(), content->name_size());
	if(fs_Fault == name) {
		type = ContentType::fault;
	} else {
		type = name.endsWith(fs_Response) ? ContentType::response : ContentType::request;

		String serviceType = String(service.objectType());
		if(!serviceType.equals(content->xmlns(), content->xmlns_size())) {
			debug_e("[UPnP] Bad namespace: %s expected, %s found", serviceType.c_str(), content->xmlns());
			return Error::BadSoapNamespace;
		}

		String ns = XML::getAttribute(content, xmlns_u);
		if(ns != serviceType) {
			debug_w("[SOAP] namespace attribute incorrect, expected '%s' but found '%s'", serviceType.c_str(),
					ns.c_str());
			return Error::BadSoapNamespace;
		}
	}

	return Error::Success;
}

void Envelope::clear()
{
	type = ContentType::none;
	content = nullptr;
	name = nullptr;
	doc.clear();
	buffer = nullptr;
}

Error Envelope::load(String&& content)
{
	clear();
	if(content.length() == 0) {
		debug_w("Service: Empty request body");
		return Error::NoSoapBody;
	}

	buffer = std::move(content);
	return parseBody();
}

Error Envelope::load(const FlashString& content)
{
	clear();
	buffer = content;
	return parseBody();
}

/*
	<s:Envelope
		xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"
		s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
		<s:Body/>
	</s:Envelope>
*/
XML::Node* Envelope::initialise(ContentType contentType)
{
	clear();

	if(contentType == ContentType::none) {
		return nullptr;
	}

	type = contentType;
	XML::insertDeclaration(doc);
	auto env = XML::appendNode(&doc, s_Envelope);
	XML::appendAttribute(env, xmlns_s, soap_namespace);
	XML::appendAttribute(env, s_encodingStyle, soap_encoding);
	return XML::appendNode(env, s_Body);
}

Envelope& Envelope::createRequest(const String& actionName)
{
	auto body = initialise(ContentType::request);

	String tag;
	tag.reserve(actionName.length() + 2);
	tag += "u:";
	tag += actionName;

	content = XML::appendNode(body, tag);
	XML::appendAttribute(content, xmlns_u, service.objectType());

	name = actionName;
	return *this;
}

Envelope& Envelope::createResponse(const String& actionName)
{
	auto body = initialise(ContentType::response);

	String tag;
	tag.reserve(actionName.length() + 10);
	tag += "u:";
	tag += actionName;
	tag += fs_Response;

	content = XML::appendNode(body, tag);
	XML::appendAttribute(content, xmlns_u, service.objectType());

	name = actionName;
	responsePrepared = true;
	return *this;
}

Envelope::Fault Envelope::createFault(ErrorCode error)
{
	auto body = initialise(ContentType::fault);

	content = XML::appendNode(body, s_Fault);
	XML::appendNode(content, fs_faultcode, s_Client);
	XML::appendNode(content, fs_faultstring, fs_UPnPError);
	auto detail = XML::appendNode(content, fs_detail);
	auto err = XML::appendNode(detail, fs_UPnPError);
	XML::appendAttribute(err, fs_xmlns, schemas_upnp_org::control_1_0);
	XML::appendNode(err, fs_errorCode, int(error));
	XML::appendNode(err, fs_errorDescription, toLongString(error));

	return fault();
}

bool Envelope::getArg(const String& name, uint32_t& value, uint32_t defaultValue)
{
	auto s = getArgValue(name);
	if(s == nullptr) {
		value = defaultValue;
		return false;
	}

	value = strtoul(s, nullptr, 10);
	return true;
}

bool Envelope::getArg(const String& name, int32_t& value, int32_t defaultValue)
{
	auto s = getArgValue(name);
	if(s == nullptr) {
		value = defaultValue;
		return false;
	}

	value = strtol(s, nullptr, 10);
	return true;
}

bool Envelope::getArg(const String& name, float& value, float defaultValue)
{
	auto s = getArgValue(name);
	if(s == nullptr) {
		value = defaultValue;
		return false;
	}

	value = strtod(s, nullptr);
	return true;
}

bool Envelope::getArg(const String& name, double& value, double defaultValue)
{
	auto s = getArgValue(name);
	if(s == nullptr) {
		value = defaultValue;
		return false;
	}

	value = strtod(s, nullptr);
	return true;
}

bool Envelope::getArg(const String& name, bool& value, bool defaultValue)
{
	auto s = getArgValue(name);
	if(s != nullptr) {
		if(strcmp(s, "1") == 0 || strcmp(s, "true") == 0 || strcmp(s, "yes") == 0) {
			value = true;
			return true;
		}

		if(strcmp(s, "0") == 0 || strcmp(s, "false") == 0 || strcmp(s, "no") == 0) {
			value = false;
			return true;
		}
	}

	debug_e("[UPnP] Bad BOOL arg '%s'", s ?: "(null)");

	value = defaultValue;
	return false;
}

String Envelope::soapAction() const
{
	String s;
	s += '"';
	s += String(service.objectType());
	s += '#';
	s += actionName();
	s += '"';
	return s;
}

String Envelope::Fault::faultCode() const
{
	if(envelope.isFault()) {
		return XML::getValue(envelope.content, fs_faultcode, "");
	} else {
		return nullptr;
	}
}

String Envelope::Fault::faultString() const
{
	if(envelope.isFault()) {
		return XML::getValue(envelope.content, fs_faultstring, "");
	} else {
		return nullptr;
	}
}

XML::Node* Envelope::Fault::detail() const
{
	return XML::getNode(envelope.content, fs_detail, "");
}

XML::Node* Envelope::Fault::error() const
{
	if(envelope.isFault()) {
		auto node = detail();
		return node ? node->first_node() : nullptr;
	} else {
		return nullptr;
	}
}

ErrorCode Envelope::Fault::errorCode() const
{
	if(envelope.isFault()) {
		return ErrorCode(XML::getValue(error(), fs_errorCode).toInt());
	} else {
		return ErrorCode::None;
	}
}

String Envelope::Fault::errorDescription() const
{
	if(envelope.isFault()) {
		return XML::getValue(error(), fs_errorDescription);
	} else {
		return nullptr;
	}
}

size_t Envelope::Fault::printTo(Print& p) const
{
	if(!envelope.isFault()) {
		return 0;
	}

	size_t n{0};

	n += p.print(fs_faultcode);
	n += p.print("=");
	n += p.println(faultCode());

	n += p.print(fs_faultstring);
	n += p.print("=");
	n += p.println(faultString());

	//	if(faultString() != fs_UPnPError) {
	//		debug_e("Unrecognised faultstring");
	//	}

	auto code = errorCode();
	n += p.print(fs_errorCode);
	n += p.print("=");
	n += p.print(int(code));
	n += p.print(' ');
	n += p.println(toString(code));

	n += p.print(fs_errorDescription);
	n += p.print("=");
	n += p.println(errorDescription());

	return n;
}

} // namespace UPnP
