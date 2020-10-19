/**
 * DeviceHost.cpp
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

#include "include/Network/UPnP/DeviceHost.h"
#include "include/Network/UPnP/DescriptionStream.h"
#include "include/Network/UPnP/ControlPoint.h"
#include <Network/SSDP/Server.h>
#include <WMath.h>
#include <Platform/Station.h>

namespace UPnP
{
DeviceHost deviceHost;

/*
 * If MX is specified, wait random time before responding.
 * This should be done by the Server to space out service responses.
 * It also allows time for previous UDP packets to be sent which helps
 * minimise transient RAM usage.
 *
 * Probably the first step is to find out how many services
 * to respond with so we can ensure all responses are sent within the
 * given time.
 *
 * Say we have 32 services (unlikely to ever be that high) with a
 * 2 second window, that gives us 62ms per response, so we'll get some
 * randomness just due to processing time.
 *
 * If there's only one service then no problem.
 *
 * Perhaps we just divide MX by the number of services and calculate a
 * random number based on that.
 *
 *
 * "For multicast M-SEARCH requests, if the search request does not contain an MX header field,
 * the device shall silently discard and ignore the search request. If the MX header field specifies
 * a field value greater than 5, the device should assume that it contained the value 5 or less."
 *
 * No reason given.
 */
void DeviceHost::onSearchRequest(const BasicMessage& request)
{
	auto mx = request["MX"];

	unsigned delay = mx ? atoi(mx) : 0;
	if(delay == 0) {
		delay = 5;
	} else if(delay > 5) {
		delay = 5;
	}
	delay = random(100, delay * 1000);

	MessageSpec ms(MESSAGE_RESPONSE);
	SearchFilter filter(ms, delay);

	filter.targetString = request["ST"];
	if(filter.targetString == SSDP::UPNP_ROOTDEVICE) {
		ms.target = TARGET_ROOT;
	} else if(filter.targetString == SSDP::SSDP_ALL) {
		ms.target = TARGET_ALL;
	} else if(filter.targetString.startsWith("urn:")) {
		ms.target = TARGET_TYPE;
	} else if(filter.targetString.startsWith("uuid:")) {
		ms.target = TARGET_UUID;
	} else {
		debug_e("[UPnP] Invalid ST field: %s", filter.targetString.c_str());
		return;
	}

	ms.remoteIP = request.remoteIP;
	ms.remotePort = request.remotePort;

	search(filter, nullptr);
}

void DeviceHost::search(SearchFilter& filter, Device* device)
{
	filter.callback = [&](Object* object, SearchMatch match) {
		auto item = new MessageSpec(filter.ms);
		item->object = object;
		item->match = match;
		server.messageQueue.add(item, filter.delayMs);
		filter.delayMs += 100;
	};

#if DEBUG_VERBOSE_LEVEL == DBG
	unsigned initialCount = server.messageQueue.count();
#endif

	if(device == nullptr) {
		for(auto dev = deviceHost.firstRootDevice(); dev != nullptr; dev = dev->getNext()) {
			dev->search(filter);
		}
	} else {
		device->search(filter);
	}

#if DEBUG_VERBOSE_LEVEL == DBG
	unsigned count = server.messageQueue.count();
	String s = toString(filter.ms.messageType);
	if(!s) {
		s = _F("**BAD**  ");
	}
	s += ' ';
	s += IpAddress(filter.ms.remoteIP).toString();
	s += ':';
	s += filter.ms.remotePort;
	s += ' ';

	if(filter.ms.messageType == MESSAGE_RESPONSE) {
		s += toString(filter.ms.target);
	} else if(filter.ms.messageType == MESSAGE_NOTIFY) {
		s += toString(filter.ms.notifySubtype);
	} else {
		s += filter.targetString;
	}
	s += _F(", queued: ");
	s += count - initialCount;
	s += _F(", total: ");
	s += count;
	s += "\r\n";

	m_puts(s.c_str());
#endif
}

void DeviceHost::notify(Device* device, NotifySubtype subtype)
{
	MessageSpec ms(subtype);
	ms.target = TARGET_ALL;
	ms.remoteIP = SSDP_MULTICAST_IP;
	ms.remotePort = SSDP_MULTICAST_PORT;
	SearchFilter filter(ms, 500);
	search(filter, device);
}

bool DeviceHost::begin()
{
	return SSDP::server.begin(
		[this](BasicMessage& msg) {
			if(msg.type == MESSAGE_MSEARCH) {
				onSearchRequest(msg);
			} else {
				for(auto cp = controlPoints.head(); cp != nullptr; cp = cp->getNext()) {
					cp->onNotify(msg);
				}
			}
		},
		[](Message& msg, MessageSpec& ms) {
			auto object = static_cast<Object*>(ms.object);
			if(object == nullptr) {
				// Send directly
				// TODO: This is ControlPoint stuff
				server.sendMessage(msg);
			} else {
				object->sendMessage(msg, ms);
			}
		});
}

void DeviceHost::end()
{
	SSDP::server.end();
}

bool DeviceHost::isActive() const
{
	return SSDP::server.isActive();
}

bool DeviceHost::registerDevice(RootDevice* device)
{
	if(!rootDevices.add(device)) {
		return false;
	}

	if(isActive()) {
		// TODO: If device already registered we should return true but not advertise
		notify(device, NTS_ALIVE);
	}

	return true;
}

bool DeviceHost::unRegisterDevice(RootDevice* device)
{
	if(!rootDevices.remove(device)) {
		// Device wasn't running
		return false;
	}

	if(isActive()) {
		notify(device, NTS_BYEBYE);
	}

	// TODO: When last notification has been sent, inform application
	return true;
}

bool DeviceHost::onHttpRequest(HttpServerConnection& connection)
{
	// Block access from remote networks, or if connected via AP
	auto remoteIP = connection.getRemoteIp();
	if(!WifiStation.isLocal(remoteIP)) {
		debug_w("[UPnP] Ignoring external request from %s", remoteIP.toString().c_str());
		return false;
	}

	auto device = rootDevices.head();
	while(device != nullptr) {
		if(device->onHttpRequest(connection)) {
			return true;
		}
		device = device->getNext();
	}
	return false;
}

} // namespace UPnP
