/**
 * Action.cpp
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
 * You should have received a copy of the GNU General Public License along with FlashString.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/Network/UPnP/Action.h"
#include "include/Network/UPnP/Service.h"

namespace UPnP
{
bool ActionInfo::load(String& content)
{
	action = nullptr;
	response = nullptr;

	if(!envelope.load(content)) {
		return false;
	}

	if(!findAction()) {
		return false;
	}

	auto ns = action->first_attribute("xmlns:u");
	return SOAP::checkAttrValue(ns, service.getField(Service::Field::serviceType));
}

bool ActionInfo::findAction()
{
	auto body = envelope.body();
	if(body == nullptr) {
		return false;
	}

	action = body->first_node();
	if(action == nullptr) {
		debug_e("[UPnP] Action missing");
		return false;
	}

	String serviceType = service.getField(Service::Field::serviceType);
	if(!SOAP::checkValue(serviceType, action->xmlns(), action->xmlns_size())) {
		action = nullptr;
		return false;
	}

	return true;
}

bool ActionInfo::getArgBool(const String& name, bool& value)
{
	auto s = actionArg(name);
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

	debug_e("[UPnP] Bad BOOL action arg '%s'", s ?: "(null)");

	return false;
}

/**
 * @brief Create a SOAP envelope for a response to the given action
 * @param action The action to respond to
 * @retval XML::Node* nullptr on error
 * @note The response uses the same XML document as the action so be sure to make
 * copies of anything you need from it before calling this method
 */
bool ActionInfo::createResponse()
{
	if(action == nullptr) {
		debug_e("[UPnP] Unexpected: action not set");
		return false;
	}

	String tag;
	tag += "u:";
	tag.concat(action->name(), action->name_size());
	tag += "Response";

	action = nullptr;
	response = nullptr;

	if(!envelope.initialise()) {
		return false;
	}

	auto body = envelope.body();
	if(body == nullptr) {
		return false;
	}

	response = XML::appendNode(body, tag);
	XML::appendAttribute(response, "xmlns:u", service.getField(Service::Field::serviceType));
	return true;
}

} // namespace UPnP
