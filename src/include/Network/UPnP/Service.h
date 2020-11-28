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
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include "Object.h"
#include "ObjectList.h"
#include "Envelope.h"
#include "Constants.h"
#include <Network/SSDP/Urn.h>

#define UPNP_SERVICE_FIELD_MAP(XX)                                                                                     \
	XX(serviceType, required)                                                                                          \
	XX(serviceId, required)                                                                                            \
	XX(SCPDURL, required)                                                                                              \
	XX(controlURL, required)                                                                                           \
	XX(eventSubURL, required)                                                                                          \
	XX(domain, custom)                                                                                                 \
	XX(type, custom)                                                                                                   \
	XX(version, custom)

namespace UPnP
{
class Device;
class Service;

/**
 * @brief Represents any kind of device, including a root device
 */
class Service : public ObjectTemplate<Service, Object>
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

	Device& root();

	String caption() const
	{
		String s;
		s += String(objectType());
		s += " {";
		s += getField(Field::serviceId);
		s += '}';
		return s;
	}

	void search(const SearchFilter& filter) override;
	bool formatMessage(Message& msg, MessageSpec& ms) override;

	bool onHttpRequest(HttpServerConnection& connection) override;

	virtual String getField(Field desc) const;

	Urn objectType() const override
	{
		return ServiceUrn(getField(Field::domain), getField(Field::type), version());
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
	 * @param env Contains the action request
	 * @retval ErrorCode
	 *
	 * Implementation should place response into `env` after reading parameters, e.g.:
	 *
	 *		if(env.actionName() == "MyAction") {
	 * 			String arg1;
	 * 			int arg2;
	 * 			env.getArg("arg1", arg1);
	 * 			env.getArg("arg2", arg2);
	 * 			auto& response = env.createResponse();
	 * 			// Process command here
	 * 			int arg3 = processMyAction(arg1, arg2);
	 * 			response.setArg("arg3", arg3);
	 * 			return ErrorCode::Success;
	 * 		}
	 *
	 * 		return ErrorCode::InvalidAction;
	 *
	 * This is usually handled by generated wrapper class templates.
	 *
	 */
	virtual Error handleAction(Envelope& env) = 0;

private:
	Device& device_;
	// actionList
	// serviceStateTable
};

} // namespace UPnP

String toString(UPnP::Service::Field field);
