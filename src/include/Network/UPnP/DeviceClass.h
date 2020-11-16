/**
 * DeviceClass.h
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

#pragma once

#include "ServiceClass.h"
#include "Device.h"
#include <Network/SSDP/Uuid.h>

namespace UPnP
{
class ControlPoint;
class DeviceControl;

/**
 * @brief Provides all information required for UPnP to construct a DeviceControl object
 */
class DeviceClass : public ClassObject
{
public:
	using List = ObjectList<DeviceClass>;
	using OwnedList = OwnedObjectList<DeviceClass>;
	using Field = Device::Field;

	Urn getUrn() const override
	{
		return DeviceUrn(getField(Field::domain), getField(Field::type), version());
	}

	virtual String getField(Field desc) const
	{
		return nullptr;
	}

	const DeviceClass* getNext() const
	{
		return reinterpret_cast<const DeviceClass*>(next());
	}

	const ServiceClass* firstService() const
	{
		return serviceClasses.head();
	}

	/**
	 * @brief When SSDP discovery notification received we pass location and USN fields here
	 * to construct a device instance.
	 * @param controlPoint Device will use this to service requests
	 * @param location URL of XML description file
	 * @param uniqueServiceName Composite of UDN and device/service type
	 * @retval DeviceControl* Constructed device object
	 */
	DeviceControl* createObject(ControlPoint& controlPoint, const char* location, const char* uniqueServiceName) const;

	template <typename Other> bool equals() const
	{
		return getField(Field::domain) == Other::domain && getField(Field::type) == Other::type &&
			   version() == Other::version_;
	}

protected:
	virtual DeviceControl* createObject(ControlPoint& controlPoint) const = 0;

	ServiceClass::List serviceClasses;
};

} // namespace UPnP
