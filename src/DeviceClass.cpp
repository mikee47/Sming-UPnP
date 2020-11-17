/**
 * DeviceClass.cpp
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

#include "include/Network/UPnP/DeviceClass.h"
#include "include/Network/UPnP/DeviceControl.h"

namespace UPnP
{
String DeviceClass::getField(Field desc) const
{
	switch(desc) {
	case Field::deviceType:
		return String(getDeviceType());
	default:
		return nullptr;
	}
}

DeviceControl* DeviceClass::createObject(ControlPoint& controlPoint, const char* location,
										 const char* uniqueServiceName) const
{
	/*
	 * Don't need the description name so throw it away.
	 *
	 * Note: UPnP 1.0 has URLBase but this is not permitted for later revisions.
	 * If ths is encountered we should alert the use.
	 */
	const char* p = strrchr(location, '/');
	String baseUrl;
	if(p == nullptr) {
		debug_e("[UPnP] Location invalid: %s", location);
		return nullptr;
	}
	baseUrl.setString(location, 1 + p - location);

	Usn usn(uniqueServiceName);
	if(usn.domain != getField(Field::domain)) {
		debug_e("[UPnP] Domain mismatch");
		return nullptr;
	}
	if(usn.kind != Usn::Kind::device || usn.type != getField(Field::type)) {
		debug_e("[UPnP] Device type mismatch");
		return nullptr;
	}
	if(usn.version != version()) {
		debug_e("[UPnP] Device version mismatch");
		return nullptr;
	}

	auto obj = createObject(controlPoint);
	obj->baseUrl_ = baseUrl;
	obj->udn_ = Uuid(usn.uuid);
	return obj;
}

} // namespace UPnP
