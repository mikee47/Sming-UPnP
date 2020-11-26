/**
 * Device.cpp
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

#include "include/Network/UPnP/Device.h"
#include "include/Network/UPnP/ItemEnumerator.h"
#include "include/Network/UPnP/DescriptionStream.h"
#include <Network/SSDP/Server.h>
#include <Network/Http/HttpConnection.h>
#include <Network/Url.h>
#include <assert.h>
#include <SystemClock.h>
#include <Platform/Station.h>
#include <FlashString/Vector.hpp>
#include <FlashString/TemplateStream.hpp>

namespace
{
IMPORT_FSTR_LOCAL(upnp_default_page, COMPONENT_PATH "/resource/default.html");
DEFINE_FSTR_LOCAL(defaultPresentationURL, "index.html");

#define XX(name, req) DEFINE_FSTR_LOCAL(fn_##name, #name);
UPNP_DEVICE_FIELD_MAP(XX);
#undef XX

#define XX(name, req) &fn_##name,
DEFINE_FSTR_VECTOR(fieldNames, FlashString, UPNP_DEVICE_FIELD_MAP(XX))
#undef XX
} // namespace

namespace UPnP
{
Device& Device::root()
{
	return isRoot() ? *this : parent_.root();
}

/*
 * Create the content. Lists are added as placeholders by DescriptionStream:
 *
 * deviceList
 * iconList
 * serviceList
 * actionList
 * serviceStateTable
 *
 * The resulting JSON is rendered into XML, then emitted in chunks.
 * When a list tag is encountered, it is built via child objects.
 *
 *
 *
 * For now, DescriptionStream 'knows' about the above lists.
 */
XML::Node* Device::getDescription(XML::Document& doc, DescType descType) const
{
	switch(descType) {
	case DescType::header: {
		auto root = XML::appendNode(&doc, "root");
		XML::appendAttribute(root, fs_xmlns, schemas_upnp_org::device_1_0);
		return root;
	}

	case DescType::content:
	case DescType::embedded: {
		String s;
		auto dev = XML::appendNode(&doc, "device");
		for(unsigned i = 0; i < unsigned(Field::customStart); ++i) {
			s = getField(Field(i));
			if(s) {
				XML::appendNode(dev, fieldNames[i], s);
			}
		}

		//		XML::appendNode(dev, "iconList");

		return dev;
	}

	default:
		return nullptr;
	}
}

String Device::getField(Field desc) const
{
	// Provide defaults for required fields
	switch(desc) {
	case Field::deviceType:
		return String(objectType());
	case Field::domain:
		return getClass().group.domain;
	case Field::type:
		return getClass().type;
	case Field::version:
		return String(version());

	case Field::presentationURL:
		return getField(Field::baseURL) + defaultPresentationURL;

	case Field::friendlyName:
	case Field::manufacturer:
	case Field::modelName:
	case Field::UDN:
		return nullptr;

	case Field::descriptionURL: {
		String url = getField(Field::baseURL);
		url += _F("desc.xml");
		return url;
	}

	case Field::baseURL: {
		String url;
		url += '/';
		url += getField(Field::type);
		url += '/';
		return url;
	}

	case Field::serverId:
		return SSDP::getServerId(getField(Field::productNameAndVersion));

	case Field::productNameAndVersion:
		return F("MyApp/1.0");
	default:
		return nullptr;
	}
}

IDataSourceStream* Device::createDescription()
{
	return new DescriptionStream(*this, root().getField(Field::descriptionURL));
}

ItemEnumerator* Device::getList(unsigned index, String& name)
{
	switch(index) {
	case 0:
		name = F("serviceList");
		return new ItemEnumerator(services_.head());
	case 1:
		name = F("deviceList");
		return new ItemEnumerator(devices_.head());
	default:
		return nullptr;
	}
}

void Device::search(const SearchFilter& filter)
{
	switch(filter.ms.target()) {
	case SearchTarget::root:
		if(isRoot()) {
			filter.callback(this, SearchMatch::root);
			return;
		}
		break;
	case SearchTarget::all:
		if(isRoot()) {
			filter.callback(this, SearchMatch::root);
		}
		filter.callback(this, SearchMatch::uuid);
		filter.callback(this, SearchMatch::type);
		break;
	case SearchTarget::type:
		if(filter.targetString == getField(Field::deviceType)) {
			filter.callback(this, SearchMatch::type);
		}
		break;
	case SearchTarget::uuid:
		if(filter.targetString == getField(Field::UDN)) {
			filter.callback(this, SearchMatch::uuid);
		}
		break;
	default:
		assert(false);
	}

	if(filter.ms.target() != SearchTarget::uuid) {
		for(auto service = services_.head(); service != nullptr; service = service->getNext()) {
			service->search(filter);
		}
	}

	for(auto device = devices_.head(); device != nullptr; device = device->getNext()) {
		device->search(filter);
	}
}

Url Device::getUrl(const String& path)
{
	return Url(URI_SCHEME_HTTP, nullptr, nullptr, WifiStation.getIP().toString(), 80, path);
}

bool Device::formatMessage(Message& msg, MessageSpec& ms)
{
	msg[HTTP_HEADER_LOCATION] = getUrl(getField(Field::descriptionURL));

	String serverId = SSDP::getServerId(getField(Field::productNameAndVersion));
	if(ms.type() == MessageType::notify) {
		msg[HTTP_HEADER_SERVER] = serverId;
	} else {
		msg[HTTP_HEADER_USER_AGENT] = serverId;
	}

	String st;
	String usn = getField(Field::UDN);
	switch(ms.match()) {
	case SearchMatch::root:
		st = SSDP::UPNP_ROOTDEVICE;
		usn += "::";
		usn += st;
		break;
	case SearchMatch::type:
		st = getField(Field::deviceType);
		usn += "::";
		usn += st;
		break;
	case SearchMatch::uuid:
		st = getField(Field::UDN);
		break;
	default:
		debug_e("[UPnP] Invalid search match value");
		return false;
	}

	if(msg.type == MessageType::notify) {
		msg["NT"] = st;
	} else {
		msg["ST"] = st;
	}
	msg["USN"] = usn;
	return true;
}

bool Device::onHttpRequest(HttpServerConnection& connection)
{
	auto request = connection.getRequest();

	if(isRoot() && request->uri.Path == getField(Field::presentationURL)) {
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

	if(request->uri.Path == getField(Field::descriptionURL)) {
		debug_i("[UPnP] Sending '%s' for '%s' to %s:%u", request->uri.Path.c_str(), getField(Field::type).c_str(),
				connection.getRemoteIp().toString().c_str(), connection.getRemotePort());
		auto response = connection.getResponse();
		if(request->method == HTTP_GET) {
			sendXml(*response, createDescription());
		} else {
			response->code = HTTP_STATUS_BAD_REQUEST;
		}
		return true;
	}

	for(auto service = services_.head(); service != nullptr; service = service->getNext()) {
		if(service->onHttpRequest(connection)) {
			return true;
		}
	}

	for(auto device = devices_.head(); device != nullptr; device = device->getNext()) {
		if(device->onHttpRequest(connection)) {
			return true;
		}
	}

	return false;
}

void Device::sendXml(HttpResponse& response, IDataSourceStream* content)
{
	response.headers[F("Content-Language")] = "en";
	response.headers[HTTP_HEADER_SERVER] = getField(Device::Field::serverId);
	response.headers[HTTP_HEADER_CONNECTION] = _F("close");
	response.headers["EXT"] = "";
	response.headers[F("X-User-Agent")] = F("Sming");
	response.sendDataStream(content, F("text/xml; charset=\"utf-8\""));
}

} // namespace UPnP

String toString(UPnP::Device::Field& field)
{
	return fieldNames[unsigned(field)];
}

bool fromString(const char* name, UPnP::Device::Field& field)
{
	int i = fieldNames.indexOf(name);
	if(i < 0) {
		return false;
	}

	field = UPnP::Device::Field(i);
	return true;
}
