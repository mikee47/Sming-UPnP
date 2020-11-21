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
 * You should have received a copy of the GNU General Public License along with Sming UPnP.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include "DeviceClass.h"
#include "ServiceClass.h"
#include "ServiceControl.h"
#include <Data/CString.h>
#include <Network/SSDP/Uuid.h>

namespace UPnP
{
class DeviceControl : public Device
{
	friend DeviceClass;

public:
	DeviceControl(const DeviceClass& deviceClass, ControlPoint& controlPoint)
		: deviceClass(deviceClass), controlPoint(controlPoint)
	{
	}

	void parseDescription(XML::Document& description);

	ServiceControl* getService(const Urn& serviceType);
	ServiceControl* getService(const String& serviceType);
	ServiceControl* getService(const ServiceClass& serviceClass);

	String getField(Field desc) const override;

	const DeviceClass& getClass() const
	{
		return deviceClass;
	}

	const String udn() const
	{
		return String(description.udn);
	}

	bool sendRequest(ActionInfo& request, const ActionInfo::Callback& callback);

	bool configure(const Url& location, XML::Document& description);

private:
	const DeviceClass& deviceClass;
	ControlPoint& controlPoint;
	ServiceControl::OwnedList services;
	struct Description {
		CString baseUrl; ///< Includes trailing path separator, e.g. "http://192.168.1.1/"
		CString udn;
		CString friendlyName;
		CString manufacturer;
		CString modelName;
		CString modelNumber;
		CString modelDescription;
		CString serialNumber;
	};
	Description description;
};

} // namespace UPnP
