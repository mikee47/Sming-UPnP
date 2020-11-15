/**
 * Object.h
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

#include "LinkedItem.h"
#include <WString.h>
#include <Delegate.h>
#include <Network/SSDP/MessageSpec.h>
#include <Data/Stream/DataSourceStream.h>
#include <Network/Http/HttpServerConnection.h>
#include <Network/Http/HttpRequest.h>
#include <Network/Http/HttpResponse.h>

namespace UPnP
{
using namespace SSDP;

class Object;
class RootDevice;

struct SearchFilter {
	using Callback = Delegate<void(Object* object, SearchMatch match)>;

	SearchFilter(const MessageSpec& ms, uint32_t delayMs) : ms(ms), delayMs(delayMs)
	{
	}

	const MessageSpec& ms; ///< Specification for message to be sent
	uint32_t delayMs;	  ///< Message delay
	String targetString;   ///< Full search target value
	Callback callback;	 ///< Called on a match
};

class Object : public LinkedItem
{
public:
	Object* getNext() const
	{
		return reinterpret_cast<Object*>(LinkedItem::next());
	}

	virtual RootDevice* getRoot() = 0;

	const RootDevice* getRoot() const
	{
		return const_cast<Object*>(this)->getRoot();
	}

	/**
	 * @brief Called during SSDP search operation
	 */
	virtual void search(const SearchFilter& filter) = 0;

	/**
	 * @brief Standard fields have been completed
	 * @note Fields can be modified typically by adding any custom fields
	 * before sending response.
	 * @param msg The message being constructed
	 * @param ms Template spec. for message
	 * @retval bool Return true to send message, false to cancel
	 */
	virtual bool formatMessage(Message& msg, MessageSpec& ms) = 0;

	/**
	 * @brief Called by framework to construct then send a message.
	 * @param msg Message to send
	 * @param ms Template spec.
	 */
	virtual void sendMessage(Message& msg, MessageSpec& ms);

	/**
	 * @brief Called by framework to handle an incoming HTTP request.
	 * @param connection
	 * @param request
	 * @param response
	 * @retval bool true if request was handled
	 */
	virtual bool onHttpRequest(HttpServerConnection& connection) = 0;

	/**
	 * @brief Called by framework to construct a device description response stream
	 * @retval IDataSourceStream* The XML description content
	 *
	 * By default, the framework generates a stream constructed from the device information fields,
	 * but this method may be overridden if, for example, a fixed description is stored in an .xml file.
	 */
	virtual IDataSourceStream* createDescription();

	/**
	 * @brief Split a device or service type string into `deviceType` and `version`
	 * @param type e.g. "Basic:1", on return gets reduced to "Basic"
	 * @retval String the version "1"
	 */
	static String splitTypeVersion(String& type);

	/**
	 * @brief Get the version section from a type string
	 * @param type e.g. "Basic:1"
	 * @retval The version "1", or nullptr if not present
	 */
	static const char* getTypeVersion(const char* type);
};

/**
 * @brief Base class template for linked items with type casting
 */
template <typename ObjectType> class ObjectTemplate : public Object
{
public:
	ObjectType* getNext() const
	{
		return reinterpret_cast<ObjectType*>(Object::next());
	}
};

} // namespace UPnP
