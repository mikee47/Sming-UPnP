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

namespace UPnP
{
class DeviceControl : public Device
{
	friend DeviceClass;

public:
	DeviceControl(const DeviceClass& deviceClass) : deviceClass(deviceClass)
	{
	}

	~DeviceControl();

	void parseDescription(XML::Document& description);

	ServiceControl* getService(const ServiceClass& serviceClass);

	String getField(Field desc) const override
	{
		return deviceClass.getField(desc) ?: Device::getField(desc);
	}

	const DeviceClass& getClass() const
	{
		return deviceClass;
	}

private:
	const DeviceClass& deviceClass;
	ServiceControl::List services;
	String location;
	String usn;
};

} // namespace UPnP
