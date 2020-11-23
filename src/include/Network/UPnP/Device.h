/**
 * Device.h
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

#include "Service.h"

#define UPNP_DEVICE_FIELD_MAP(XX)                                                                                      \
	XX(deviceType, required)                                                                                           \
	XX(friendlyName, required)                                                                                         \
	XX(manufacturer, required)                                                                                         \
	XX(manufacturerURL, allowed)                                                                                       \
	XX(modelDescription, recommended)                                                                                  \
	XX(modelName, required)                                                                                            \
	XX(modelNumber, recommended)                                                                                       \
	XX(modelURL, allowed)                                                                                              \
	XX(serialNumber, recommended)                                                                                      \
	XX(UDN, required)                                                                                                  \
	XX(presentationURL, recommended)                                                                                   \
	XX(domain, custom)                                                                                                 \
	XX(type, custom_required)                                                                                          \
	XX(version, custom)                                                                                                \
	XX(serverId, custom)                                                                                               \
	XX(baseURL, custom)                                                                                                \
	XX(descriptionURL, custom)

namespace UPnP
{
class Device;
class Request;

/**
 * @brief Represents any kind of device, including a root device
 */
class Device : public ObjectTemplate<Device>
{
public:
	enum class Field {
#define XX(name, req) name,
		UPNP_DEVICE_FIELD_MAP(XX)
#undef XX
			customStart = domain,
		MAX
	};

	using List = ObjectList<Device>;
	using OwnedList = OwnedObjectList<Device>;

	Device(Device& parent) : parent_(parent)
	{
	}

	Device(Device* parent) : parent_(*(parent ?: this))
	{
	}

	String caption() const
	{
		String s;
		s += friendlyName();
		s += " {";
		s += getField(Field::UDN);
		s += '}';
		return s;
	}

	RootDevice& root() const;

	bool isRoot() const
	{
		return static_cast<const Device*>(this) == &parent_;
	}

	void search(const SearchFilter& filter) override;
	bool formatMessage(Message& msg, MessageSpec& ms) override;

	virtual String getField(Field desc) const;

	String deviceType() const
	{
		return getField(Field::deviceType);
	}

	String friendlyName() const
	{
		return getField(Field::friendlyName);
	}

	bool onHttpRequest(HttpServerConnection& connection) override;

	void addDevice(Device* device)
	{
		devices_.add(device);
	}

	void addService(Service* service)
	{
		services_.add(service);
	}

	XML::Node* getDescription(XML::Document& doc, DescType descType) const override;

	IDataSourceStream* createDescription() override;

	ItemEnumerator* getList(unsigned index, String& name) override;

	void sendXml(HttpResponse& response, IDataSourceStream* content);

	Service* firstService()
	{
		return services_.head();
	}

	Device* firstDevice()
	{
		return devices_.head();
	}

	Device& parent() const
	{
		return parent_;
	}

private:
	Device& parent_;
	Service::OwnedList services_;
	Device::OwnedList devices_;
};

} // namespace UPnP

String toString(UPnP::Device::Field& field);

bool fromString(const char* name, UPnP::Device::Field& field);

inline bool fromString(const String& name, UPnP::Device::Field& field)
{
	return fromString(name.c_str(), field);
}
