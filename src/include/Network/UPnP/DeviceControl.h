/**
 * DeviceControl.h
 *
 * Copyright 2020 mikee47 <mike@sillyhouse.net>
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

#include "Device.h"
#include "ServiceControl.h"
#include <Data/CString.h>
#include <Network/SSDP/Uuid.h>

namespace UPnP
{
class ControlPoint;

class DeviceControl : public Device
{
public:
	using List = ObjectList<DeviceControl>;
	using OwnedList = OwnedObjectList<DeviceControl>;

	struct Description {
		CString udn;
		CString friendlyName;
		CString manufacturer;
		CString modelName;
		CString modelNumber;
		CString modelDescription;
		CString serialNumber;
	};

	DeviceControl(DeviceControl& parent) : Device(parent)
	{
	}

	DeviceControl(DeviceControl* parent = nullptr) : Device(parent)
	{
	}

	/**
	 * @brief Called on root device only during discovery
	 */
	bool configureRoot(ControlPoint& controlPoint, const String& location, XML::Node* device);

	DeviceControl& root()
	{
		return reinterpret_cast<DeviceControl&>(Device::root());
	}

	const DeviceControl& root() const
	{
		return reinterpret_cast<const DeviceControl&>(Device::root());
	}

	String getUrl(const String& path) const override
	{
		return String(root().rootConfig->baseUrl) + path;
	}

	String getUrlBasePath() const override
	{
		return root().rootConfig->basePath.c_str();
	}

	ControlPoint& controlPoint() const
	{
		return root().rootConfig->controlPoint;
	}

	template <typename T> ServiceControl* getService(const T& serviceType)
	{
		return Device::getService<ServiceControl>(serviceType);
	}

	template <typename T> DeviceControl* getDevice(const T& deviceType)
	{
		return Device::getDevice<DeviceControl>(deviceType);
	}

	String getField(Field desc) const override;

	const String udn() const
	{
		return String(description_.udn);
	}

	bool configure(XML::Node* device);

	DeviceControl* getNext()
	{
		return reinterpret_cast<DeviceControl*>(next());
	}

	DeviceControl& parent()
	{
		return reinterpret_cast<DeviceControl&>(Device::parent());
	}

	Description& description()
	{
		return description_;
	}

protected:
	Description description_;

	struct RootConfig {
		ControlPoint& controlPoint;
		CString baseUrl;  ///< e.g. "http://192.168.1.1:80"
		CString basePath; ///< Includes trailing path separator, e.g. "/devices/1/"
	};
	std::unique_ptr<RootConfig> rootConfig;
};

} // namespace UPnP
