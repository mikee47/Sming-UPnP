/**
 * Object.cpp
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
 * You should have received a copy of the GNU General Public License along with Sming UPnP.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/Network/UPnP/Object.h"
#include "include/Network/UPnP/DescriptionStream.h"
#include "include/Network/UPnP/DeviceHost.h"
#include <Network/SSDP/Server.h>

namespace UPnP
{
void Object::sendMessage(Message& msg, MessageSpec& ms)
{
	if(formatMessage(msg, ms)) {
		server.sendMessage(msg);
	}
}

} // namespace UPnP
