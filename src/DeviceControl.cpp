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

namespace UPnP
{
String DeviceControl::getField(Field desc) const
{
	switch(desc) {
	case Field::UDN:
		return toString(udn_);
	case Field::baseURL:
		return baseUrl_;
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

} // namespace UPnP
