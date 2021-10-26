/****
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
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include "BaseObject.h"
#include "ObjectClass.h"
#include <WString.h>
#include <Delegate.h>
#include <Network/SSDP/MessageSpec.h>
#include <Data/Stream/DataSourceStream.h>
#include <Network/Http/HttpServerConnection.h>
#include <Network/Http/HttpRequest.h>
#include <Network/Http/HttpResponse.h>
#include <Network/SSDP/Urn.h>

namespace UPnP
{
class Object;

class Object : public BaseObject
{
public:
	using Version = ObjectClass::Version;

	virtual const ObjectClass& getClass() const = 0;

	Object* getNext() const
	{
		return reinterpret_cast<Object*>(LinkedItem::next());
	}

	bool typeIs(const Urn& objectType) const
	{
		return objectType == this->objectType();
	}

	bool typeIs(const String& objectType) const
	{
		return this->objectType() == objectType;
	}

	bool typeIs(const ObjectClass& objectClass) const
	{
		return typeIs(objectClass.objectType());
	}

	virtual Urn objectType() const
	{
		return getClass().objectType();
	}

	virtual Version version() const
	{
		return getClass().version();
	}

	/**
	 * @brief Called during SSDP search operation
	 */
	virtual void search(const SearchFilter& filter) = 0;

	/**
	 * @brief Called by framework to handle an incoming HTTP request.
	 * @param connection
	 * @param request
	 * @param response
	 * @retval bool true if request was handled
	 */
	virtual bool onHttpRequest(HttpServerConnection& connection)
	{
		return false;
	}

	/**
	 * @brief Called by framework to construct a device description response stream
	 * @retval IDataSourceStream* The XML description content
	 *
	 * By default, the framework generates a stream constructed from the device information fields,
	 * but this method may be overridden if, for example, a fixed description is stored in an .xml file.
	 */
	virtual IDataSourceStream* createDescription()
	{
		return nullptr;
	}
};

} // namespace UPnP
