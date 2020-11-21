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

#include "ObjectClass.h"

namespace UPnP
{
class RootDeviceControl;
class ControlPoint;

/**
 * @brief Provides information required for UPnP to construct a DeviceControl object
 */
struct DeviceClass : public ObjectClass {
	/**
	 * @brief When SSDP discovery notification received we pass location and USN fields here
	 * to construct a device instance.
	 * @param controlPoint Device will use this to service requests
	 * @param location URL of XML description file
	 * @param uniqueServiceName Composite of UDN and device/service type
	 * @param description Root device description
	 * @retval RootDeviceControl* Constructed device object
	 */
	RootDeviceControl* createRootObject(ControlPoint& controlPoint, const Url& location,
										const String& uniqueServiceName, XML::Document& description) const;
};

} // namespace UPnP
