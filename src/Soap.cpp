/**
 * Soap.cpp
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
 * You should have received a copy of the GNU General Public License along with Sming UPnP.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/Network/UPnP/Soap.h"

namespace SOAP
{
namespace
{
DEFINE_FSTR_LOCAL(soap_namespace, "http://schemas.xmlsoap.org/soap/envelope/");
DEFINE_FSTR_LOCAL(soap_encoding, "http://schemas.xmlsoap.org/soap/encoding/");
} // namespace

bool checkValue(const char* str1, size_t len1, const char* str2, size_t len2)
{
	if(len1 == len2 && str1 != nullptr && str2 != nullptr && memcmp(str1, str2, len1) == 0) {
		return true;
	}

	debug_e("[SOAP] %s expected, %s found", str1, str2);
	return false;
}

bool checkNodeName(XML::Node* node, const String& str)
{
	if(node == nullptr) {
		debug_e("[SOAP] %s missing", str.c_str());
		return false;
	}
	return checkValue(str, node->name(), node->name_size());
}

bool checkAttrValue(XML::Attribute* attr, const String& str)
{
	if(attr == nullptr) {
		debug_e("[SOAP] attr missing (looking for %s)", str.c_str());
		return false;
	}
	return checkValue(str, attr->value(), attr->value_size());
}

XML::Node* getNode(XML::Node* parent, const String& name)
{
	return parent ? parent->first_node(name.c_str(), "", name.length()) : nullptr;
}

String getNodeValue(XML::Node* parent, const String& name)
{
	auto node = getNode(parent, name);
	return node ? String(node->value(), node->value_size()) : nullptr;
}

XML::Node* addNodeValue(XML::Node* parent, const String& name, const String& value)
{
	if(parent == nullptr) {
		return nullptr;
	}

	return XML::appendNode(parent, name, value);
}

XML::Node* Envelope::findEnvelope()
{
	// Skip declaration if present
	String ns(soap_namespace);
	auto env = doc.first_node("Envelope", ns.c_str(), 8, ns.length());
	if(env == nullptr) {
		debug_e("[SOAP] Envelope missing");
	}
	return env;
}

XML::Node* Envelope::findBody()
{
	auto env = findEnvelope();
	if(env == nullptr) {
		return nullptr;
	}

	auto body = env->first_node("Body", nullptr, 4);
	if(body == nullptr) {
		debug_e("[SOAP] Body missing");
	}

	return body;

	//	auto env = doc.first_node();
	//	if(!checkValue(F("Envelope"), env)) {
	//		return nullptr;
	//	}

	//	auto body = env->first_node();
	//	if(!checkValue(F("Body"), body)) {
	//		return nullptr;
	//	}

	return body;
}

bool Envelope::load(String& content)
{
	body_ = nullptr;

	if(!XML::deserialize(doc, content)) {
		return false;
	}

	body_ = findBody();
	return body_ != nullptr;
}

bool Envelope::load(const FlashString& content)
{
	body_ = nullptr;

	if(!XML::deserialize(doc, content)) {
		return false;
	}

	body_ = findBody();
	return body_ != nullptr;
}

/*
	<s:Envelope
		xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"
		s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
		<s:Body/>
	</s:Envelope>
 */
bool Envelope::initialise()
{
	doc.clear();
	XML::insertDeclaration(doc);
	auto env = XML::appendNode(&doc, "s:Envelope");
	XML::appendAttribute(env, "xmlns:s", soap_namespace);
	XML::appendAttribute(env, "s:encodingStyle", soap_encoding);

	body_ = XML::appendNode(env, "s:Body");

	return true;
}

} // namespace SOAP
