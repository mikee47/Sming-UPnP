/**
 * RootDevice.cpp
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

#include "include/Network/UPnP/RootDevice.h"
#include <Network/SSDP/Server.h>
#include <FlashString/TemplateStream.hpp>
#include <Platform/Station.h>
#include <SmingVersion.h>

IMPORT_FSTR(upnp_default_page, COMPONENT_PATH "/resource/default.html");

namespace UPnP
{
DEFINE_FSTR_LOCAL(defaultPresentationURL, "index.html");

Url RootDevice::getURL(const String& path)
{
	return Url(URI_SCHEME_HTTP, nullptr, nullptr, WifiStation.getIP().toString(), tcpPort, path);
}

String RootDevice::getField(Field desc) const
{
	switch(desc) {
	case Field::presentationURL:
		return getField(Field::baseURL) + defaultPresentationURL;
	case Field::serverId:
		return SSDP::SERVER_ID;
	case Field::baseURL:
		return "/";

	default:
		return Device::getField(desc);
	}
}

bool RootDevice::onHttpRequest(HttpServerConnection& connection)
{
	auto request = connection.getRequest();
	if(request->uri.Path == getField(Field::presentationURL)) {
		debug_i("[UPnP] Sending default presentation page for '%s'", getField(Field::type).c_str());
		auto response = connection.getResponse();
		auto tmpl = new FSTR::TemplateStream(upnp_default_page);
		tmpl->onGetValue([this](const char* name) -> String {
			Field field;
			String s;
			if(fromString(name, field)) {
				s = getField(field);
			}
			return s;
		});
		response->sendDataStream(tmpl, MIME_HTML);
		return true;
	}

	debug_i("[UPnP] URL not matched: %s", request->uri.Path.c_str());
	return Device::onHttpRequest(connection);
}

void RootDevice::search(const SearchFilter& filter)
{
	if(filter.ms.target() == SearchTarget::root) {
		filter.callback(this, SearchMatch::root);
		return;
	}

	if(filter.ms.target() == SearchTarget::all) {
		filter.callback(this, SearchMatch::root);
	}

	Device::search(filter);
}

} // namespace UPnP
