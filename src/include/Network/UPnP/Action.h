/**
 * Action.h
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

#pragma once

#include <Network/Http/HttpConnection.h>
#include "Soap.h"

namespace UPnP
{
class Service;

class ActionInfo
{
public:
	ActionInfo(HttpConnection& connection, Service& service) : connection(connection), service(service)
	{
	}

	/**
	 * @brief Parse request content into document
	 * @param request MUST remain for the lifetime of action parsing
	 */
	bool load(String& request);

	/**
	 * @brief Discard action and create empty response document
	 */
	bool createResponse();

	const char* actionArg(const String& name)
	{
		return SOAP::getNodeValue(action, name);
	}

	bool getArgBool(const String& name, bool& value);

	bool actionIs(const FlashString& name) const
	{
		return action ? name.equals(action->name(), action->name_size()) : false;
	}

	bool actionIs(const String& name) const
	{
		return action ? name.equals(action->name()) : false;
	}

	String actionName() const
	{
		if(action == nullptr) {
			return nullptr;
		}
		return String(action->name(), action->name_size());
	}

public:
	HttpConnection& connection;
	Service& service;
	SOAP::Envelope envelope;
	XML::Node* action{nullptr};
	XML::Node* response{nullptr};

private:
	bool findAction();
};

} // namespace UPnP
