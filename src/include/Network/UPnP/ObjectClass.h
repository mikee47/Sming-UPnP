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
	using Kind = Urn::Kind;

	/**
	 * @brief Interface version number
	 */
	using Version = uint8_t;

	/**
	 * @brief Object constructor function
	 */
	using CreateObject = Object* (*)(DeviceControl* owner);

	struct Device {
		const FlashString* friendlyName;
		const FlashString* manufacturer;
		const FlashString* manufacturerURL;
		const FlashString* modelDescription;
		const FlashString* modelName;
		const FlashString* modelNumber;
		const FlashString* modelURL;
		const FlashString* serialNumber;
		const FlashString* UDN;
	};

	struct Service {
		const FlashString* serviceId;
		const FlashString* schema;
	};

	Kind kind_;
	Version version_;
	const FlashString* domain_;
	const FlashString* type_;
	const CreateObject createObject_;
	// Both of these fields are optional and may be null
	union {
		const Device* device_;
		const Service* service_;
	};

	const FlashString& domain() const
	{
		return *domain_;
	}

	const FlashString& type() const
	{
		return *type_;
	}

	const Device* device() const
	{
		assert(kind() == Kind::device);
		return device_;
	}

	const Service* service() const
	{
		assert(kind() == Kind::service);
		return service_;
	}

	Version version() const
	{
		return FSTR::readValue(&version_);
	}

	Kind kind() const
	{
		return FSTR::readValue(&kind_);
	}

	DeviceControl* createRootDevice() const
	{
		assert(kind() == Kind::device);
		return reinterpret_cast<DeviceControl*>(createObject_(nullptr));
	}

	DeviceControl* createDevice(DeviceControl& owner) const
	{
		assert(kind() == Kind::device);
		return reinterpret_cast<DeviceControl*>(createObject_(&owner));
	}

	ServiceControl* createService(DeviceControl& owner) const
	{
		assert(kind() == Kind::service);
		return reinterpret_cast<ServiceControl*>(createObject_(&owner));
	}

	Urn objectType() const;
	bool operator==(const ObjectClass& other) const;
	bool typeIs(const Urn& objectType) const;
	bool typeIs(Urn::Kind kind, const String& type, uint8_t version) const;

	explicit operator bool() const
	{
		// We're always valid :-)
		return true;
	}
};

} // namespace UPnP
