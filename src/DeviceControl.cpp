/**
 * DeviceControl.cpp
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

#include "include/Network/UPnP/DeviceControl.h"
#include "include/Network/UPnP/ControlPoint.h"

namespace UPnP
{
bool DeviceControl::configureRoot(ControlPoint& controlPoint, const String& location, XML::Node* device)
{
	Url baseUrl(location);
	String path = std::move(baseUrl.Path);
	int i = path.lastIndexOf('/');
	if(i >= 0) {
		path.setLength(i);
	}
	rootConfig.reset(new RootConfig{controlPoint, baseUrl.toString(), path});

	return DeviceControl::configure(device);
}

bool DeviceControl::configure(XML::Node* device)
{
	description_.udn = XML::getValue(device, _F("UDN"));
	description_.friendlyName = XML::getValue(device, _F("friendlyName"));
	description_.manufacturer = XML::getValue(device, _F("manufacturer"));
	description_.modelDescription = XML::getValue(device, _F("modelDescription"));
	description_.modelName = XML::getValue(device, _F("modelName"));
	description_.modelNumber = XML::getValue(device, _F("modelNumber"));
	description_.serialNumber = XML::getValue(device, _F("serialNumber"));

	return true;
}

String DeviceControl::getField(Field desc) const
{
	switch(desc) {
	case Field::UDN:
		return String(description_.udn);
	case Field::friendlyName:
		return String(description_.friendlyName);
	case Field::manufacturer:
		return String(description_.manufacturer);
	case Field::modelDescription:
		return String(description_.modelDescription);
	case Field::modelName:
		return String(description_.modelName);
	case Field::modelNumber:
		return String(description_.modelNumber);
	case Field::serialNumber:
		return String(description_.serialNumber);
	default:
		return Device::getField(desc);
	}
}

bool DeviceControl::sendRequest(Envelope& request, const Envelope::Callback& callback)
{
	return controlPoint().sendRequest(request, callback);
}

} // namespace UPnP
