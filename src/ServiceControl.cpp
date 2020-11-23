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
#include "include/Network/UPnP/RootDeviceControl.h"

namespace UPnP
{
bool ServiceControl::configure(const XML::Node* service)
{
	// Omit leading separator as device baseURL includes it
	auto getUrl = [&](const String& name) {
		String url = XML::getValue(service, name);
		if(!url) {
			debug_e("[UPnP] %s missing from %s", name.c_str(), serviceType().c_str());
		} else if(url[0] == '/') {
			url.remove(0, 1);
		}
		return url;
	};

	description.controlURL = getUrl(F("controlURL"));
	description.eventSubURL = getUrl(F("eventSubURL"));
	description.serviceId = XML::getValue(service, F("serviceId"));

	debug_i("[UPnP] controlURL = %s", description.controlURL.c_str());

	return true;
}

String ServiceControl::getField(Field desc) const
{
	switch(desc) {
	case Field::domain:
		return getClass().group.domain;
	case Field::type:
		return getClass().type;
	case Field::version:
		return String(version());
	case Field::serviceId:
		return description.serviceId.c_str();
	case Field::baseURL:
		return device().getField(Device::Field::baseURL);
	case Field::controlURL:
		if(description.controlURL) {
			return getField(Field::baseURL) + description.controlURL.c_str();
		} else {
			return nullptr;
		}
	case Field::eventSubURL:
		if(description.eventSubURL) {
			return getField(Field::baseURL) + description.eventSubURL.c_str();
		} else {
			return nullptr;
		}
	default:
		return Service::getField(desc);
	}
}

bool ServiceControl::sendRequest(ActionInfo& request, const ActionInfo::Callback& callback)
{
	return root().sendRequest(request, callback);
}

} // namespace UPnP
