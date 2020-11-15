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

namespace UPnP
{
class DeviceControl;

/**
 * @brief Provides all information required for UPnP to construct a DeviceControl object
 */
class DeviceClass : public ClassObject
{
public:
	using Field = Device::Field;

	Urn getUrn() const override
	{
		return DeviceUrn(getField(Field::domain), getField(Field::type), getField(Field::version));
	}

	virtual String getField(Field desc) const
	{
		return nullptr;
	}

	DeviceControl* createObject(XML::Document& description) const;

protected:
	virtual DeviceControl* createObject() const = 0;

	ServiceClass::List serviceClasses;
};

} // namespace UPnP
