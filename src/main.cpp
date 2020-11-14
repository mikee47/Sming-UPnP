/**
 * main.cpp
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

#include "include/Network/UPnP/DeviceHost.h"
#include "include/Network/UPnP/ControlPoint.h"
#include <Network/SSDP/Server.h>

namespace UPnP
{
static bool initialized;

bool initialize()
{
	if(!initialized) {
		initialized = SSDP::server.begin(
			[](BasicMessage& msg) {
				if(msg.type == MessageType::msearch) {
					deviceHost.onSearchRequest(msg);
				} else {
					ControlPoint::onSsdpMessage(msg);
				}
			},
			[](Message& msg, MessageSpec& ms) {
				auto object = ms.object<Object>();
				if(object == nullptr) {
					server.sendMessage(msg);
				} else {
					object->sendMessage(msg, ms);
				}
			});
	}

	return initialized;
}

void finalize()
{
	if(initialized) {
		SSDP::server.end();
		initialized = false;
	}
}

} // namespace UPnP
