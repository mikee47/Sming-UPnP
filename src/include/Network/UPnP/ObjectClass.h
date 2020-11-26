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

#include <Network/SSDP/Urn.h>
#include <WString.h>
#include <FlashString/Vector.hpp>
#include <assert.h>

namespace UPnP
{
class Object;
class DeviceControl;
class ServiceControl;

struct ObjectClass {
	using List = const FSTR::Vector<ObjectClass>&;

	/**
	 * @brief Interface version number
	 */
	using Version = uint8_t;

	/**
	 * @brief Object constructor function
	 */
	using CreateObject = Object* (*)(DeviceControl* owner);

	const FlashString* domain_;
	const FlashString* type_;
	const FlashString* schema_;
	Version version_;
	Urn::Kind kind_;
	const CreateObject createObject_;

	const FlashString& domain() const
	{
		return *domain_;
	}

	const FlashString& type() const
	{
		return *type_;
	}

	const FlashString& schema() const
	{
		return *schema_;
	}

	Version version() const
	{
		return FSTR::readValue(&version_);
	}

	Urn::Kind kind() const
	{
		return FSTR::readValue(&kind_);
	}

	DeviceControl* createRootDevice() const
	{
		assert(kind() == Urn::Kind::device);
		return reinterpret_cast<DeviceControl*>(createObject_(nullptr));
	}

	DeviceControl* createDevice(DeviceControl& owner) const
	{
		assert(kind() == Urn::Kind::device);
		return reinterpret_cast<DeviceControl*>(createObject_(&owner));
	}

	ServiceControl* createService(DeviceControl& owner) const
	{
		assert(kind() == Urn::Kind::service);
		return reinterpret_cast<ServiceControl*>(createObject_(&owner));
	}

	Urn objectType() const;
	bool operator==(const ObjectClass& other) const;
	bool typeIs(const Urn& objectType) const;
	bool typeIs(const String& type, uint8_t version) const;

	explicit operator bool() const
	{
		// We're always valid :-)
		return true;
	}
};

} // namespace UPnP
