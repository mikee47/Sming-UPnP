/****
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
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include "Device.h"

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

	bool registerDevice(Device* device);

	bool unRegisterDevice(Device* device);

	bool onHttpRequest(HttpServerConnection& connection);

	/**
	 * @brief Create an HTML page which applications may serve up to assist with debugging
	 */
	IDataSourceStream* generateDebugPage(const String& title);

	Device::List& devices()
	{
		return devices_;
	}

	void notify(Device* device, NotifySubtype subype);

	/**
	 * @brief Called via SSDP when incoming message received
	 */
	void onSearchRequest(const BasicMessage& request);

private:
	void search(SearchFilter& filter, Device* device);

	Device::List devices_;
};

extern DeviceHost deviceHost;

} // namespace UPnP
