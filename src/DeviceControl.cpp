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

#include "include/Network/UPnP/DeviceControl.h"
#include "include/Network/UPnP/ControlPoint.h"

namespace UPnP
{
bool DeviceControl::configure(const Url& location, XML::Document& description)
{
	auto device = XML::getNode(description, _F("/device"));
	if(device == nullptr) {
		debug_e("[UPnP] device node not found");
		return false;
	}

	auto getValue = [&](const char* name) -> String {
		auto node = device->first_node(name);
		if(node == nullptr) {
			return nullptr;
		}
		return String(node->value(), node->value_size());
	};

	this->description.baseUrl = location.toString();
	this->description.udn = getValue(_F("UDN"));
	this->description.friendlyName = getValue(_F("friendlyName"));
	this->description.manufacturer = getValue(_F("manufacturer"));
	this->description.modelDescription = getValue(_F("modelDescription"));
	this->description.modelName = getValue(_F("modelName"));
	this->description.modelNumber = getValue(_F("modelNumber"));
	this->description.serialNumber = getValue(_F("serialNumber"));
	return true;
}

String DeviceControl::getField(Field desc) const
{
	switch(desc) {
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
		return String(description.baseUrl);
	default:
		String s = deviceClass.getField(desc);
		return s ?: Device::getField(desc);
	}
}

ServiceControl* DeviceControl::getService(const ServiceClass& serviceClass)
{
	ServiceControl* service;
	for(service = services.head(); service != nullptr; service = service->getNext()) {
		if(service->getClass() == serviceClass) {
			return service;
		}
	}

	const ServiceClass* cls;
	for(cls = deviceClass.firstService(); cls != nullptr; cls = cls->getNext()) {
		auto c1 = static_cast<const ServiceClass*>(cls);
		auto c2 = static_cast<const ServiceClass*>(&serviceClass);
		if(c1 == c2) {
			//		if(*cls == serviceClass) {
			auto service = cls->createObject(*this, *cls);
			services.add(service);
			return service;
		}
	}
	return nullptr;
}

bool DeviceControl::sendRequest(ActionInfo& request, const ActionInfo::Callback& callback)
{
	return controlPoint.sendRequest(request, callback);
}

} // namespace UPnP
