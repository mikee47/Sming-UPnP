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

	Url baseUrl(location);
	baseUrl.Path = nullptr;
	this->description.baseUrl = baseUrl.toString();
	this->description.udn = XML::getValue(device, _F("UDN"));
	this->description.friendlyName = XML::getValue(device, _F("friendlyName"));
	this->description.manufacturer = XML::getValue(device, _F("manufacturer"));
	this->description.modelDescription = XML::getValue(device, _F("modelDescription"));
	this->description.modelName = XML::getValue(device, _F("modelName"));
	this->description.modelNumber = XML::getValue(device, _F("modelNumber"));
	this->description.serialNumber = XML::getValue(device, _F("serialNumber"));

	auto serviceList = device->first_node("serviceList");
	debug_i("serviceList %sfound", serviceList ? "" : "NOT ");
	if(serviceList != nullptr) {
		auto svc = serviceList->first_node();
		while(svc != nullptr) {
			String svcType = XML::getValue(svc, _F("serviceType"));
			auto service = getService(svcType);
			if(service == nullptr) {
				debug_w("[UPnP] Service not found in device class: %s", svcType.c_str());
			} else {
				service->configure(svc);
			}
			svc = svc->next_sibling();
		}
	}

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

ServiceControl* DeviceControl::getService(const Urn& serviceType)
{
	ServiceControl* service;
	for(service = services.head(); service != nullptr; service = service->getNext()) {
		if(service->getClass().typeIs(serviceType)) {
			return service;
		}
	}

	for(auto cls = deviceClass.firstService(); cls != nullptr; cls = cls->getNext()) {
		if(cls->typeIs(serviceType)) {
			auto service = cls->createObject(*this, *cls);
			services.add(service);
			return service;
		}
	}

	return nullptr;
}

ServiceControl* DeviceControl::getService(const String& serviceType)
{
	Urn urn;
	if(!urn.decompose(serviceType)) {
		debug_e("** Invalid serviceType: %s", serviceType.c_str());
		return nullptr;
	}

	return getService(urn);
}

ServiceControl* DeviceControl::getService(const ServiceClass& serviceClass)
{
	ServiceControl* service;
	for(service = services.head(); service != nullptr; service = service->getNext()) {
		if(service->getClass() == serviceClass) {
			return service;
		}
	}

	for(auto cls = deviceClass.firstService(); cls != nullptr; cls = cls->getNext()) {
		if(*cls == serviceClass) {
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
