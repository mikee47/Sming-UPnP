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
 * You should have received a copy of the GNU General Public License along with FlashString.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/Network/UPnP/Object.h"
#include "include/Network/UPnP/DescriptionStream.h"
#include "include/Network/UPnP/DeviceHost.h"
#include <Network/SSDP/Server.h>

namespace UPnP
{
String Object::splitTypeVersion(String& type)
{
	String ver;
	int i = type.indexOf(':');
	if(i >= 0) {
		ver = type.substring(i + 1);
		type.remove(i);
	}
	return ver;
}

const char* Object::getTypeVersion(const char* type)
{
	auto p = strchr(type, ':');
	if(p != nullptr) {
		++p;
	}
	return p;
}

void Object::sendMessage(Message& msg, MessageSpec& ms)
{
	if(formatMessage(msg, ms)) {
		server.sendMessage(msg);
	}
}

IDataSourceStream* Object::createDescription()
{
	return new DescriptionStream(this);
}

} // namespace UPnP
