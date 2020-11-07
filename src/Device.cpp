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
 * You should have received a copy of the GNU General Public License along with FlashString.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/Network/UPnP/RootDevice.h"
#include "include/Network/UPnP/ItemEnumerator.h"
#include "include/Network/UPnP/DescriptionStream.h"
#include <Network/Http/HttpConnection.h>
#include <Network/Url.h>
#include <assert.h>
#include <SystemClock.h>
#include <FlashString/Vector.hpp>

namespace
{
#define XX(name, req) DEFINE_FSTR_LOCAL(fn_##name, #name);
UPNP_DEVICE_FIELD_MAP(XX);
#undef XX

#define XX(name, req) &fn_##name,
DEFINE_FSTR_VECTOR(fieldNames, FlashString, UPNP_DEVICE_FIELD_MAP(XX))
#undef XX
} // namespace

namespace UPnP
{
RootDevice* Device::getRoot()
{
	assert(parent_ != nullptr);
	Device* dev = this;
	while(dev->parent_ != nullptr) {
		dev = dev->parent_;
	}
	return dev->getRoot();
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
XML::Node* Device::getDescription(XML::Document& doc, DescType descType)
{
	switch(descType) {
	case DescType::header: {
		auto root = XML::appendNode(&doc, "root");
		XML::appendAttribute(root, "xmlns", F("urn:schemas-upnp-org:device-1-0"));
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

String Device::getField(Field desc)
{
	// Provide defaults for required fields
	switch(desc) {
	case Field::deviceType:
		return DeviceUrn(getField(Field::domain), getField(Field::type), getField(Field::version));
	case Field::type:
		return DeviceType::Basic;
	case Field::friendlyName:
	case Field::manufacturer:
	case Field::modelName:
	case Field::UDN:
		return F("REQUIRED FIELD");
	case Field::domain:
		return schemas_upnp_org;
	case Field::descriptionURL: {
		String url = getField(Field::baseURL);
		url += _F("desc.xml");
		return url;
	}
	case Field::baseURL: {
		String url = getRoot()->getField(desc);
		String s = getField(Field::type);
		splitTypeVersion(s);
		url += s;
		url += '/';
		return url;
	}
	case Field::serverId:
		return (parent_ == nullptr) ? nullptr : getRoot()->getField(desc);
	default:
		return nullptr;
	}
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
	case SearchTarget::all:
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

bool Device::formatMessage(Message& msg, MessageSpec& ms)
{
	msg[HTTP_HEADER_SERVER] = getField(Field::serverId);
	msg[HTTP_HEADER_LOCATION] = getRoot()->getURL(getField(Field::descriptionURL));

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
