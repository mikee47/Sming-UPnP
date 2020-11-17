/**
 * ServiceControl.cpp
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

#include "include/Network/UPnP/ServiceControl.h"
#include "include/Network/UPnP/DeviceControl.h"

namespace UPnP
{
String ServiceControl::getField(Field desc) const
{
	switch(desc) {
	case Field::baseURL:
		return device.getField(Device::Field::baseURL);
	case Field::controlURL:
		return getField(Field::baseURL) + serviceClass.getField(desc);
	case Field::eventSubURL:
		return getField(Field::baseURL) + serviceClass.getField(desc);
	default:
		String s = serviceClass.getField(desc);
		return s ?: Service::getField(desc);
	}
}

bool ServiceControl::sendRequest(ActionInfo& request, const ActionInfo::Callback& callback)
{
	return device.sendRequest(request, callback);
}

} // namespace UPnP
