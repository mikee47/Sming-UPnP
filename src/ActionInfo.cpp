/**
 * ActionInfo.cpp
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

#include "include/Network/UPnP/ActionInfo.h"
#include "include/Network/UPnP/Service.h"

namespace UPnP
{
bool ActionInfo::load(String& request)
{
	content = nullptr;

	if(!envelope.load(request)) {
		return false;
	}

	if(!findAction()) {
		return false;
	}

	auto ns = content->first_attribute("xmlns:u");
	return SOAP::checkAttrValue(ns, service.getField(Service::Field::serviceType));
}

bool ActionInfo::findAction()
{
	auto body = envelope.body();
	if(body == nullptr) {
		return false;
	}

	content = body->first_node();
	if(content == nullptr) {
		debug_e("[UPnP] Action missing");
		return false;
	}

	String serviceType = service.getField(Service::Field::serviceType);
	if(!SOAP::checkValue(serviceType, content->xmlns(), content->xmlns_size())) {
		content = nullptr;
		return false;
	}

	name.setString(content->name(), content->name_size());
	return true;
}

bool ActionInfo::getArg(const String& name, uint32_t& value, uint32_t defaultValue)
{
	auto s = getArgValue(name);
	if(s == nullptr) {
		value = defaultValue;
		return false;
	}

	value = strtoul(s, nullptr, 10);
	return true;
}

bool ActionInfo::getArg(const String& name, int32_t& value, int32_t defaultValue)
{
	auto s = getArgValue(name);
	if(s == nullptr) {
		value = defaultValue;
		return false;
	}

	value = strtol(s, nullptr, 10);
	return true;
}

bool ActionInfo::getArg(const String& name, float& value, float defaultValue)
{
	auto s = getArgValue(name);
	if(s == nullptr) {
		value = defaultValue;
		return false;
	}

	value = strtod(s, nullptr);
	return true;
}

bool ActionInfo::getArg(const String& name, double& value, double defaultValue)
{
	auto s = getArgValue(name);
	if(s == nullptr) {
		value = defaultValue;
		return false;
	}

	value = strtod(s, nullptr);
	return true;
}

bool ActionInfo::getArg(const String& name, bool& value, bool defaultValue)
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

bool ActionInfo::createRequest(const String& name)
{
	this->name = name;

	String tag;
	tag += "u:";
	tag += name;

	content = nullptr;

	if(!envelope.initialise()) {
		return false;
	}

	auto body = envelope.body();
	if(body == nullptr) {
		return false;
	}

	content = XML::appendNode(body, tag);
	XML::appendAttribute(content, "xmlns:u", service.getField(Service::Field::serviceType));
	return true;
}

bool ActionInfo::createResponse()
{
	if(content == nullptr) {
		debug_e("[UPnP] Unexpected: request not set");
		return false;
	}

	String tag;
	tag += "u:";
	tag.concat(content->name(), content->name_size());
	tag += "Response";

	content = nullptr;

	if(!envelope.initialise()) {
		return false;
	}

	auto body = envelope.body();
	if(body == nullptr) {
		return false;
	}

	content = XML::appendNode(body, tag);
	XML::appendAttribute(content, "xmlns:u", service.getField(Service::Field::serviceType));
	return true;
}

} // namespace UPnP
