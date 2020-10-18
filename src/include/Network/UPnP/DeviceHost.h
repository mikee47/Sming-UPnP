/**
 * DeviceHost.h
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
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
 * You should have received a copy of the GNU General Public License along with FlashString.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include "RootDevice.h"
#include "ControlPoint.h"

namespace UPnP
{
class DeviceHost
{
public:
	/**
	 * @brief Applications must call this to initialise UPnP stack
	 */
	bool begin();

	void end();

	bool isActive() const;

	bool registerDevice(RootDevice* device);

	bool unRegisterDevice(RootDevice* device);

	bool registerControlPoint(ControlPoint* cp)
	{
		return controlPoints.add(cp);
	}

	bool unRegisterControlPoint(ControlPoint* cp)
	{
		return controlPoints.remove(cp);
	}

	bool onHttpRequest(HttpServerConnection& connection);

	RootDevice* firstRootDevice()
	{
		return rootDevices.head();
	}

	void notify(Device* device, NotifySubtype subype);

private:
	void onSearchRequest(const BasicMessage& request);

	void search(SearchFilter& filter, Device* device);

private:
	RootDeviceList rootDevices;
	ControlPointList controlPoints;
};

extern DeviceHost deviceHost;

} // namespace UPnP
