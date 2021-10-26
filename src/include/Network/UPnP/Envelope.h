/****
 * Envelope.h
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

#pragma once

#include <RapidXML.h>
#include "Error.h"
#include "ErrorCode.h"
#include "Base64.h"

namespace UPnP
{
using Error = UPnP::Error;
class Service;

/**
 * @brief Class to manage a SOAP envelope for service request/response
 */
class Envelope
{
public:
	enum class ContentType {
		none,
		fault,
		request,
		response,
	};

	class Fault
	{
	public:
		Fault(Envelope& envelope) : envelope(envelope)
		{
		}

		explicit operator bool() const
		{
			return envelope.isFault();
		}

		String faultCode() const;
		String faultString() const;
		ErrorCode errorCode() const;
		String errorDescription() const;

		size_t printTo(Print& p) const;

	private:
		Envelope& envelope;
		XML::Node* detail() const;
		XML::Node* error() const;
	};

	Envelope(const Service& service) : service(service)
	{
	}

	/**
	 * @brief Wipe the envelope contents
	 */
	void clear();

	bool isEmpty() const
	{
		return type == ContentType::none;
	}

	bool isFault() const
	{
		return type == ContentType::fault;
	}

	/**
	 * @brief Load a SOAP document
	 * @param content MUST remain valid for the lifetime of this Envelope,
	 * or until initialise() is called.
	 * @{
	 */
	Error load(String&& content);

	Error load(const FlashString& content);
	/** @} */

	/**
	 * @brief Obtain content as XML string
	 */
	String serialize(bool pretty)
	{
		prepareResponse();
		return XML::serialize(doc, pretty);
	}

	/**
	 * @brief Serialize XML content to a stream
	 */
	size_t serialize(Print& p, bool pretty)
	{
		prepareResponse();
		return XML::serialize(doc, p, pretty);
	}

	/**
	 * @brief Get the current envelope content type
	 */
	ContentType contentType() const
	{
		return type;
	}

	/**
	 * @brief Get the action name
	 */
	String actionName() const
	{
		return name;
	}

	/**
	 * @brief Initialise the envelope as a request
	 */
	Envelope& createRequest(const String& actionName);

	/**
	 * @brief Initialise the envelope as a response
	 */
	Envelope& createResponse(const String& actionName);

	/**
	 * @brief Set a flag that this should be converted to Response on next setArg() call
	 */
	void convertToResponse()
	{
		if(type == ContentType::request) {
			type = ContentType::response;
			responsePrepared = false;
		}
	}

	/**
	 * @brief If Response is required but hasn't been prepared yet, do it now.
	 * This wipes out the incoming request.
	 */
	void prepareResponse()
	{
		if(type == Envelope::ContentType::response && !responsePrepared) {
			createResponse(actionName());
		}
	}

	/**
	 * @brief Initialise the envelope as a fault
	 */
	Fault createFault(ErrorCode error);

	/**
	 * @name Fetch fault object
	 * @{
	 */
	Fault fault()
	{
		return Fault(*this);
	}

	const Fault fault() const
	{
		return Fault(*const_cast<Envelope*>(this));
	}

	/** @} */

	/**
	 * @name Argument getters
	 * @{
	 */
	const char* getArgValue(const String& name) const
	{
		auto node = XML::getNode(content, name, "");
		return node ? node->value() : nullptr;
	}

	String getArg(const String& name) const
	{
		return XML::getValue(content, name, "");
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

	bool getArg(const String& name, Base64& value)
	{
		value = getArgValue(name);
		return bool(value);
	}

	/** @} */

	/**
	 * @name Argument setters
	 * @{
	 */
	bool addArg(const String& name, const String& value)
	{
		prepareResponse();
		assert(type == ContentType::request || type == ContentType::response);
		XML::appendNode(content, name, value);
		return true;
	}

	bool addArg(const String& name, const char* value)
	{
		return addArg(name, String(value));
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

	bool addArg(const String& name, const Base64& value)
	{
		return addArg(name, value.encode());
	}

	/** @} */

	String soapAction() const;

	const Service& service;

private:
	Error parseBody();
	Error verifyObjectType(const String& objectType) const;
	XML::Node* initialise(ContentType contentType);

	XML::Document doc;
	String buffer;
	String name;
	XML::Node* content{nullptr};
	ContentType type{};
	Error lastError{};
	bool responsePrepared{false};
};

} // namespace UPnP
