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
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/Network/UPnP/ServiceControl.h"
#include "include/Network/UPnP/DeviceControl.h"

namespace UPnP
{
bool ServiceControl::configure(const XML::Node* service)
{
	description_.controlURL = XML::getValue(service, F("controlURL"));
	description_.eventSubURL = XML::getValue(service, F("eventSubURL"));
	description_.serviceId = XML::getValue(service, F("serviceId"));

	debug_i("[UPnP] controlURL = %s", description_.controlURL.c_str());

	return true;
}

String ServiceControl::getField(Field desc) const
{
	switch(desc) {
	case Field::serviceId:
		return description_.serviceId.c_str();

	case Field::controlURL:
		return description_.controlURL.c_str();

	case Field::eventSubURL:
		return description_.eventSubURL.c_str();

	default:
		return Service::getField(desc);
	}
}

bool ServiceControl::sendRequest(Envelope& request, const Envelope::Callback& callback)
{
	return device().sendRequest(request, callback);
}

} // namespace UPnP
