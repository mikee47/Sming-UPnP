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
 * You should have received a copy of the GNU General Public License along with Sming UPnP.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/Network/UPnP/DeviceControl.h"
#include "include/Network/UPnP/ControlPoint.h"

namespace UPnP
{
bool DeviceControl::configure(XML::Node* device)
{
	this->description.udn = XML::getValue(device, _F("UDN"));
	this->description.friendlyName = XML::getValue(device, _F("friendlyName"));
	this->description.manufacturer = XML::getValue(device, _F("manufacturer"));
	this->description.modelDescription = XML::getValue(device, _F("modelDescription"));
	this->description.modelName = XML::getValue(device, _F("modelName"));
	this->description.modelNumber = XML::getValue(device, _F("modelNumber"));
	this->description.serialNumber = XML::getValue(device, _F("serialNumber"));

	return true;
}

String DeviceControl::getField(Field desc) const
{
	switch(desc) {
	case Field::domain:
		return getClass().group.domain;
	case Field::type:
		return getClass().type;
	case Field::version:
		return String(version());
	case Field::UDN:
		return String(description.udn);
	case Field::friendlyName:
		return String(description.friendlyName);
	case Field::manufacturer:
		return String(description.manufacturer);
	case Field::modelDescription:
		return String(description.modelDescription);
	case Field::modelName:
		return String(description.modelName);
	case Field::modelNumber:
		return String(description.modelNumber);
	case Field::serialNumber:
		return String(description.serialNumber);
	case Field::baseURL:
		return root().baseURL();
	default:
		return Device::getField(desc);
	}
}

ServiceControl* DeviceControl::getService(const Urn& serviceType)
{
	if(!serviceType) {
		debug_e("[UPnP] DeviceControl::getService(): Invalid serviceType");
		return nullptr;
	}

	for(auto service = firstService(); service != nullptr; service = service->getNext()) {
		if(service->getClass().typeIs(serviceType)) {
			return service;
		}
	}

	auto cls = ControlPoint::findServiceClass(serviceType);
	if(cls == nullptr) {
		return nullptr;
	}

	auto service = cls->createService(*this);
	addService(service);
	return service;
}

DeviceControl* DeviceControl::getDevice(const Urn& deviceType)
{
	if(!deviceType) {
		debug_e("[UPnP] DeviceControl::getDevice(): Invalid deviceType");
		return nullptr;
	}

	for(auto device = firstDevice(); device != nullptr; device = device->getNext()) {
		if(device->getClass().typeIs(deviceType)) {
			return device;
		}
	}

	const ObjectClass* cls = ControlPoint::findDeviceClass(deviceType);
	if(cls == nullptr) {
		return nullptr;
	}

	auto device = cls->createDevice(*this);
	addDevice(device);
	return device;
}

} // namespace UPnP
