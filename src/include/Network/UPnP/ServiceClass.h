/**
 * ServiceClass.h
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

#include "ClassObject.h"
#include "Service.h"

namespace UPnP
{
class DeviceControl;
class ServiceControl;
class DeviceClass;

/**
 * @brief Provides all information required for UPnP to construct a ServiceControl object
 */
class ServiceClass : public ClassObject
{
public:
	using Field = Service::Field;

	ServiceClass(const DeviceClass& deviceClass) : devcls(deviceClass)
	{
	}

	Urn getUrn() const override
	{
		return ServiceUrn(getField(Field::domain), getField(Field::type), getField(Field::version));
	}

	virtual String getField(Field desc) const
	{
		return nullptr;
	}

	const ServiceClass* getNext() const
	{
		return reinterpret_cast<const ServiceClass*>(next());
	}

	const DeviceClass& deviceClass() const
	{
		return devcls;
	}

	virtual ServiceControl* createObject(DeviceControl& device) const = 0;

private:
	const DeviceClass& devcls;
};

} // namespace UPnP
