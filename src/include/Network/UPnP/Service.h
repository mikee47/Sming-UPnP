/**
 * Service.h
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

#include "Object.h"
#include "ObjectList.h"
#include "ActionInfo.h"
#include "Constants.h"
#include <Network/SSDP/Urn.h>

#define UPNP_SERVICE_FIELD_MAP(XX)                                                                                     \
	XX(serviceType, required)                                                                                          \
	XX(serviceId, required)                                                                                            \
	XX(SCPDURL, required)                                                                                              \
	XX(controlURL, required)                                                                                           \
	XX(eventSubURL, required)                                                                                          \
	XX(domain, custom)                                                                                                 \
	XX(baseURL, custom)                                                                                                \
	XX(type, custom)                                                                                                   \
	XX(version, custom)

namespace UPnP
{
class Device;
class Service;

/**
 * @brief Represents any kind of device, including a root device
 */
class Service : public ObjectTemplate<Service>
{
public:
	enum class Field {
#define XX(name, req) name,
		UPNP_SERVICE_FIELD_MAP(XX)
#undef XX
			customStart = domain,
		MAX
	};

	using List = ObjectList<Service>;
	using OwnedList = OwnedObjectList<Service>;

	Service(Device& device) : device_(device)
	{
	}

	RootDevice& root() const;

	String caption() const
	{
		String s;
		s += getField(Field::serviceType);
		s += " {";
		s += getField(Field::serviceId);
		s += '}';
		return s;
	}

	void search(const SearchFilter& filter) override;
	bool formatMessage(Message& msg, MessageSpec& ms) override;

	bool onHttpRequest(HttpServerConnection& connection) override;

	virtual String getField(Field desc) const;

	String serviceType() const
	{
		return getField(Field::serviceType);
	}

	String serviceId() const
	{
		return getField(Field::serviceId);
	}

	XML::Node* getDescription(XML::Document& doc, DescType descType) const override;

	IDataSourceStream* createDescription() override;

	ItemEnumerator* getList(unsigned index, String& name) override;

	Device& device() const
	{
		return device_;
	}

	/**
	 * @brief An action request has been received
	 * @todo We need to define actions for a service, accessed via enumerator.
	 * We can also then parse the arguments and check for validity, then pass this
	 * information to `invoke()`, which is code generated for each service type.
	 * The user then gets a set of methods to implement for their service.
	 */
	virtual void handleAction(ActionInfo& info) = 0;

private:
	Device& device_;
	// actionList
	// serviceStateTable
};

} // namespace UPnP

String toString(UPnP::Service::Field field);
