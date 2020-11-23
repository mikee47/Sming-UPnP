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

#include "DeviceClass.h"
#include "Device.h"
#include "ServiceClass.h"
#include "ServiceControl.h"
#include <Data/CString.h>
#include <Network/SSDP/Uuid.h>

namespace UPnP
{
class RootDeviceControl;
class ControlPoint;

class DeviceControl : public Device
{
public:
	DeviceControl(DeviceControl& parent) : Device(parent)
	{
	}

	DeviceControl(DeviceControl* parent) : Device(parent)
	{
	}

	RootDeviceControl& root() const
	{
		return reinterpret_cast<RootDeviceControl&>(Device::root());
	}

	ServiceControl* getService(const Urn& serviceType);

	ServiceControl* getService(const String& serviceType)
	{
		return getService(Urn(serviceType));
	}

	ServiceControl* getService(const ServiceClass& serviceClass)
	{
		return getService(serviceClass.objectType());
	}

	template <class Control> Control* getService()
	{
		return reinterpret_cast<Control*>(getService(Control(*this).getClass().objectType()));
	}

	DeviceControl* getDevice(const Urn& deviceType);

	DeviceControl* getDevice(const String& deviceType)
	{
		return getDevice(Urn(deviceType));
	}

	DeviceControl* getDevice(const DeviceClass& deviceClass)
	{
		return getDevice(deviceClass.objectType());
	}

	template <class Control> Control* getDevice()
	{
		return reinterpret_cast<Control*>(getDevice(Control(*this).getClass().objectType()));
	}

	String getField(Field desc) const override;

	virtual const DeviceClass& getClass() const = 0;

	Version version() const override
	{
		return getClass().version();
	}

	const String udn() const
	{
		return String(description.udn);
	}

	bool configure(XML::Node* device);

	ServiceControl* firstService()
	{
		return reinterpret_cast<ServiceControl*>(Device::firstService());
	}

	DeviceControl* firstDevice()
	{
		return reinterpret_cast<DeviceControl*>(Device::firstDevice());
	}

	DeviceControl* getNext()
	{
		return reinterpret_cast<DeviceControl*>(next());
	}

	DeviceControl& parent() const
	{
		return reinterpret_cast<DeviceControl&>(Device::parent());
	}

protected:
	struct Description {
		CString udn;
		CString friendlyName;
		CString manufacturer;
		CString modelName;
		CString modelNumber;
		CString modelDescription;
		CString serialNumber;
	};
	Description description;

	struct RootConfig {
		ControlPoint& controlPoint;
		CString baseUrl; ///< Includes trailing path separator, e.g. "http://192.168.1.1/"
	};
	std::unique_ptr<RootConfig> rootConfig;
};

} // namespace UPnP
