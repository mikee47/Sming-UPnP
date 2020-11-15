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
#include <Data/CString.h>

namespace UPnP
{
class DeviceControl : public Device
{
public:
	DeviceControl(const DeviceClass& deviceClass) : cls(deviceClass)
	{
	}

	void parseDescription(XML::Document& description);

	ServiceControl* getService(const ServiceClass& serviceClass);

private:
	const DeviceClass& cls;
	CString friendlyName_;
	CString udn_;
};

} // namespace UPnP
