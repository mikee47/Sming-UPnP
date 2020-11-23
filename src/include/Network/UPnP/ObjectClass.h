/**
 * ObjectClass.h
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

#pragma once

#include "Object.h"
#include <Network/SSDP/Urn.h>
#include <WString.h>
#include <WVector.h>
#include <FlashString/Vector.hpp>

namespace UPnP
{
struct ClassGroup;
class RootDeviceControl;
class DeviceControl;
class ServiceControl;

struct ObjectClass {
	using CreateObject = Object* (*)(DeviceControl* owner);

	const ClassGroup& group;
	const FlashString& type;
	Object::Version version_;
	Urn::Kind kind_;
	CreateObject createObject;

	uint8_t version() const
	{
		return FSTR::readValue(&version_);
	}

	Urn::Kind kind() const
	{
		return FSTR::readValue(&kind_);
	}

	RootDeviceControl* createRootDevice() const
	{
		return (kind() == Urn::Kind::device) ? reinterpret_cast<RootDeviceControl*>(createObject(nullptr)) : nullptr;
	}

	DeviceControl* createDevice(DeviceControl& owner) const
	{
		return (kind() == Urn::Kind::device) ? reinterpret_cast<DeviceControl*>(createObject(&owner)) : nullptr;
	}

	ServiceControl* createService(DeviceControl& owner) const
	{
		return (kind() == Urn::Kind::service) ? reinterpret_cast<ServiceControl*>(createObject(&owner)) : nullptr;
	}

	Urn objectType() const;
	bool operator==(const ObjectClass& other) const;
	bool typeIs(const Urn& objectType) const;
	bool typeIs(const String& type, uint8_t version) const;
};

struct ClassGroup {
	using List = Vector<const ClassGroup*>;

	const FlashString& domain;
	const FSTR::Vector<ObjectClass>& classes;

	const ObjectClass* find(const Urn& objectType) const;
	const ObjectClass* find(const String& type, uint8_t version) const;
};

} // namespace UPnP
