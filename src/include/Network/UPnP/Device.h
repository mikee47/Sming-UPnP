/****
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
 * You should have received a copy of the GNU General Public License along with this library.
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
	XX(productNameAndVersion, custom)                                                                                  \
	XX(URLBase, deprecated)                                                                                            \
	XX(descriptionURL, custom)

namespace UPnP
{
struct SpecVersion {
	uint8_t major;
	uint8_t minor;
};

class Device;
class Request;

/**
 * @brief Represents any kind of device, including a root device
 */
class Device : public ObjectTemplate<Device, Object>
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

	Device(Device* parent = nullptr) : parent_(*(parent ?: this))
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

	/**
	 * @brief Get fully qualified URL
	 */
	virtual String getUrl(const String& path) const;

	/**
	 * @brief Get the base URL path
	 */
	virtual String getUrlBasePath() const;

	/**
	 * @brief Resolve a path (relative or absolute) into an absolute path
	 */
	String resolvePath(const String& path) const;

	Device& root();

	const Device& root() const
	{
		return const_cast<Device*>(this)->root();
	}

	bool isRoot() const
	{
		return this == &parent_;
	}

	template <class S, typename T> S* getService(const T& serviceType)
	{
		return reinterpret_cast<S*>(services_.find(serviceType));
	}

	template <typename T> Service* getService(const T& serviceType)
	{
		return getService<Service>(serviceType);
	}

	template <class D, typename T> D* getDevice(const T& deviceType)
	{
		return reinterpret_cast<D*>(devices_.find(deviceType));
	}

	template <typename T> Device* getDevice(const T& deviceType)
	{
		return getDevice<Device>(deviceType);
	}

	void search(const SearchFilter& filter) override;
	bool formatMessage(Message& msg, MessageSpec& ms) override;

	virtual String getField(Field desc) const;

	Urn objectType() const override
	{
		return DeviceUrn(getField(Field::domain), getField(Field::type), version());
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

	Device& parent()
	{
		return parent_;
	}

	Service::OwnedList& services()
	{
		return services_;
	}

	OwnedList& devices()
	{
		return devices_;
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
