/**
 * ActionInfo.h
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

#pragma once

#include <Network/Http/HttpConnection.h>
#include "Soap.h"

namespace UPnP
{
class Service;

class ActionInfo
{
public:
	using Callback = Delegate<void(ActionInfo& info)>;

	ActionInfo(const Service& service) : service(service)
	{
	}

	/**
	 * @brief Parse request content into document
	 * @param request MUST remain for the lifetime of action parsing
	 */
	bool load(String& request);

	/**
	 * @brief Prepare for new outgoing request
	 */
	bool createRequest(const String& name);

	/**
	 * @brief Prepare for new outgoing response. Request will be discarded.
	 */
	bool createResponse();

	/**
	 * @name Argument getters
	 * @{
	 */
	String getArg(const String& name)
	{
		return SOAP::getNodeValue(content, name);
	}

	bool getArg(const String& name, char& value, char defaultValue = '?')
	{
		auto s = getArgValue(name);
		if(s == nullptr) {
			value = defaultValue;
			return false;
		}

		value = *s;
		return true;
	}

	bool getArg(const String& name, String& value)
	{
		value = getArg(name);
		return bool(value);
	}

	template <typename T>
	typename std::enable_if<std::is_unsigned<T>::value && !std::is_floating_point<T>::value, bool>::type
	getArg(const String& name, T& value, T defaultValue = T{})
	{
		uint32_t n;
		if(!getArg(name, n, defaultValue)) {
			value = defaultValue;
			return false;
		}

		value = n;
		return true;
	}

	template <typename T>
	typename std::enable_if<std::is_signed<T>::value && !std::is_floating_point<T>::value, bool>::type
	getArg(const String& name, T& value, T defaultValue = T{})
	{
		int32_t n;
		if(!getArg(name, n, defaultValue)) {
			value = defaultValue;
			return false;
		}

		value = n;
		return true;
	}

	bool getArg(const String& name, bool& value, bool defaultValue = false);
	bool getArg(const String& name, uint32_t& value, uint32_t defaultValue = 0);
	bool getArg(const String& name, int32_t& value, int32_t defaultValue = 0);
	bool getArg(const String& name, float& value, float defaultValue = 0.0);
	bool getArg(const String& name, double& value, double defaultValue = 0.0);

	/** @} */

	/**
	 * @name Argument setters
	 * @{
	 */
	bool addArg(const String& name, const char* value)
	{
		return SOAP::addNodeValue(content, name, value);
	}

	bool addArg(const String& name, const String& value)
	{
		return SOAP::addNodeValue(content, name, value);
	}

	bool addArg(const String& name, char value)
	{
		return addArg(name, String(value));
	}

	bool addArg(const String& name, uint32_t value)
	{
		return addArg(name, String(value));
	}

	bool addArg(const String& name, int value)
	{
		return addArg(name, String(value));
	}

	bool addArg(const String& name, bool value)
	{
		return addArg(name, value ? "1" : "0");
	}

	bool addArg(const String& name, float value)
	{
		return addArg(name, String(value));
	}

	bool addArg(const String& name, double value)
	{
		return addArg(name, String(value));
	}

	/** @} */

	bool actionIs(const FlashString& name) const
	{
		return content ? name.equals(content->name(), content->name_size()) : false;
	}

	bool actionIs(const String& name) const
	{
		return content ? name.equals(content->name()) : false;
	}

	String actionName() const
	{
		return name;
	}

	explicit operator bool() const
	{
		return content != nullptr;
	}

	String toString(bool pretty) const
	{
		return envelope.toString(pretty);
	}

	size_t serialize(ReadWriteStream& stream, bool pretty)
	{
		return XML::serialize(envelope.doc, stream, pretty);
	}

	const Service& service;

private:
	char* getArgValue(const String& name)
	{
		auto node = SOAP::getNode(content, name);
		return node ? node->value() : nullptr;
	}

	bool findAction();

	SOAP::Envelope envelope;
	String name;
	XML::Node* content{nullptr};
};

} // namespace UPnP
