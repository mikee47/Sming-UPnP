/**
 * RootDeviceControl.cpp
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

#include "include/Network/UPnP/DeviceControl.h"
#include "include/Network/UPnP/ControlPoint.h"

namespace UPnP
{
bool RootDeviceControl::configure(ControlPoint& controlPoint, const Url& location, XML::Document& description)
{
	auto device = XML::getNode(description, _F("/device"));
	if(device == nullptr) {
		debug_e("[UPnP] device node not found");
		return false;
	}

	Url baseUrl(location);
	baseUrl.Path = nullptr;
	rootConfig.reset(new RootConfig{controlPoint, baseUrl.toString()});

	return DeviceControl::configure(device);
}

String RootDeviceControl::getField(Field desc) const
{
	switch(desc) {
	case Field::baseURL:
		return String(rootConfig->baseUrl);
	default:
		return Device::getField(desc);
	}
}

bool RootDeviceControl::sendRequest(ActionInfo& request, const ActionInfo::Callback& callback)
{
	return rootConfig->controlPoint.sendRequest(request, callback);
}

} // namespace UPnP
