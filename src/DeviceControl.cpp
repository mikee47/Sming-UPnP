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
void DeviceControl::parseDescription(XML::Document& description)
{
	auto get = [&](const String& path) -> String {
		auto node = XML::getNode(description, path);
		if(node == nullptr) {
			debug_e("[UPnP] Not found: %s", path.c_str());
			return nullptr;
		}
		return node->value();
	};

	friendlyName_ = get(F("/device/friendlyName"));
	udn_ = get(F("/device/UDN"));

	//	for(auto dc = cls;)
}

ServiceControl* DeviceControl::getService(const ServiceClass& serviceClass)
{
	return nullptr;
}

String DeviceControl::getField(Field desc) const
{
	switch(desc) {
	case Field::friendlyName:
		return String(friendlyName_);
	case Field::UDN:
		return String(udn_);
	default:
		return cls.getField(desc) ?: Device::getField(desc);
	}
}

} // namespace UPnP
