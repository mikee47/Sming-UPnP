/**
 * RootDevice.h
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

#include "Device.h"
#include <SSDP/Message.h>

namespace UPnP
{
struct SpecVersion {
	uint8_t major;
	uint8_t minor;
};

class RootDevice : public Device
{
public:
	virtual SpecVersion getSpecVersion()
	{
		return {1, 0};
	}

	RootDevice* getRoot() override
	{
		return this;
	}

	Url getURL(const String& path);

	String getField(Field desc) override;

	bool onHttpRequest(HttpServerConnection& connection) override;

	void search(const SearchFilter& filter) override;

	RootDevice* getNext()
	{
		return reinterpret_cast<RootDevice*>(Device::next());
	}

	/*
	 * Set port that HTTP requests will be received on
	 */
	void setTcpPort(uint16_t port)
	{
		tcpPort = port;
	}

	uint16_t getTcpPort() const
	{
		return tcpPort;
	}

private:
	uint16_t tcpPort = 80;
};

using RootDeviceList = ObjectList<RootDevice>;

} // namespace UPnP
