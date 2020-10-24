/**
 * Service.cpp
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
#include <Data/Stream/MemoryDataStream.h>
#include <Data/Stream/FlashMemoryStream.h>
#include <RapidXML.h>
#include <Network/SSDP/UUID.h>

namespace UPnP
{
#define XX(name, req) DEFINE_FSTR_LOCAL(fn_##name, #name);
UPNP_SERVICE_FIELD_MAP(XX);
#undef XX

static FSTR_TABLE(fieldNames) = {
#define XX(name, req) FSTR_PTR(fn_##name),
	UPNP_SERVICE_FIELD_MAP(XX)
#undef XX
};

RootDevice* Service::getRoot()
{
	return (device_ == nullptr) ? nullptr : device_->getRoot();
}

XML::Node* Service::getDescription(XML::Document& doc, DescType descType)
{
	switch(descType) {
	case DescType::header: {
		auto root = XML::appendNode(&doc, "scpd");
		XML::appendAttribute(root, _F("xmlns"), _F("urn:schemas-upnp-org:service-1-0"));
		return root;
	}

	case DescType::embedded: {
		String s;
		auto service = XML::appendNode(&doc, _F("service"));
		for(unsigned i = 0; i < unsigned(Field::customStart); ++i) {
			s = getField(Field(i));
			if(s) {
				XML::appendNode(service, *fieldNames[i], s);
			}
		}
		return service;
	}

	case DescType::content: {
		// TODO
		return nullptr;
	}

	default:
		return nullptr;
	}
}

String Service::getField(Field desc)
{
	// Provide defaults for required fields
	switch(desc) {
	case Field::serviceType: {
		String s = F("urn:");
		s += getField(Field::domain);
		s += _F(":service:");
		s += getField(Field::type);
		return s;
	}

	case Field::type:
	case Field::serviceId:
		return F("REQUIRED FIELD");

	case Field::SCPDURL:
		return getField(Field::baseURL) + _F("desc.xml");

	case Field::controlURL:
		return getField(Field::baseURL) + _F("control");

	case Field::eventSubURL:
		return getField(Field::baseURL) + _F("event");

	case Field::domain:
		return device_->getField(Device::Field::domain);

	case Field::baseURL: {
		String url = device_->getField(Device::Field::baseURL);
		String s = getField(Field::type);
		splitTypeVersion(s);
		url += s;
		url += '/';
		return url;
	}

	default:
		return nullptr;
	}
}

ItemEnumerator* Service::getList(unsigned index, String& name)
{
	switch(index) {
	// These ones will be virtual lists because of their size
	//	case 0:
	//		name = F("actionList");
	//		return new ItemEnumerator(actionList...
	//	case 1:
	//		name = F("serviceStateTable");
	//		return new ItemEnumerator(serviceStateTable...
	default:
		return nullptr;
	}
}

void Service::search(const SearchFilter& filter)
{
	switch(filter.ms.target) {
	case TARGET_ALL:
		filter.callback(this, MATCH_TYPE);
		break;
	case TARGET_TYPE:
		if(filter.targetString == getField(Field::serviceType)) {
			filter.callback(this, MATCH_TYPE);
		}
		break;
	default:
		assert(false);
	}
}

bool Service::formatMessage(Message& msg, MessageSpec& ms)
{
	if(ms.match != MATCH_TYPE) {
		debug_e("[UPnP] Invalid search match value");
		return false;
	}

	msg[HTTP_HEADER_SERVER] = device_->getField(Device::Field::serverId);
	msg[HTTP_HEADER_LOCATION] = getRoot()->getURL(getField(Field::SCPDURL)).toString();

	String st = getField(Field::serviceType);
	String usn = device_->getField(Device::Field::UDN);
	usn += "::";
	usn += st;

	if(msg.type == MESSAGE_NOTIFY) {
		msg["NT"] = st;
	} else {
		msg["ST"] = st;
	}
	msg["USN"] = usn;
	return true;
}

bool Service::onHttpRequest(HttpServerConnection& connection)
{
	auto& request = *connection.getRequest();
	auto& response = *connection.getResponse();

	auto& uri = request.uri;

	auto printRequest = [&](bool verbose = false) {
		debug_i("[UPnP] %s:%u %s %s for '%s'", connection.getRemoteIp().toString().c_str(), connection.getRemotePort(),
				http_method_str(request.method), uri.Path.c_str(), getField(Field::type).c_str());

#if DEBUG_VERBOSE_LEVEL >= DBG
		if(verbose) {
			auto& headers = request.headers;
			for(unsigned i = 0; i < headers.count(); ++i) {
				m_puts("  ");
				m_puts(headers[i].c_str());
			}
		}
#endif
	};

	auto handleControl = [&]() {
		String req = request.getBody();
		if(req.length() == 0) {
			debug_w("Service: Empty request body");
			return;
		}

#if DEBUG_VERBOSE_LEVEL >= DBG
		m_puts(req.c_str());
		m_puts("\r\n");
#endif

		ActionInfo info(connection, *this);
		if(!info.load(req)) {
			return;
		}

		String actionName = info.actionName();
		handleAction(info);

		if(info.response == nullptr) {
			debug_w("[UPnP] Unhandled action: %s", actionName.c_str());
			info.createResponse();
			// TODO: Set an error code in the response
		}

		auto stream = new MemoryDataStream;
		XML::serialize(info.envelope.doc, stream);
		device_->sendXml(response, stream);

#if DEBUG_VERBOSE_LEVEL >= DBG
		String s;
		XML::serialize(info.envelope.doc, s, true);
		m_puts(s.c_str());
		m_puts("\r\n");
#endif
	};

	auto handleSubscribe = [&]() {
		UUID uuid;
		uuid.generate();

		response.headers[HTTP_HEADER_SERVER] = device_->getField(Device::Field::serverId);
		response.headers["SID"] = String("uuid:") + String(uuid);
		response.headers[HTTP_HEADER_CONTENT_LENGTH] = "0";
		response.headers["TIMEOUT"] = "1800";
		response.code = HTTP_STATUS_OK;
	};

	if(uri.Path == getField(Field::SCPDURL)) {
		printRequest();
		if(request.method == HTTP_GET) {
			device_->sendXml(response, createDescription());
		} else {
			response.code = HTTP_STATUS_BAD_REQUEST;
		}
		return true;
	}

	if(uri.Path == getField(Field::controlURL)) {
		printRequest();
		if(request.method == HTTP_POST) {
			handleControl();
		} else {
			response.code = HTTP_STATUS_BAD_REQUEST;
		}
		return true;
	}

	if(uri.Path == getField(Field::eventSubURL)) {
		printRequest(true);
		// TODO: Handle this URL
		if(request.method == HTTP_SUBSCRIBE || request.method == HTTP_UNSUBSCRIBE) {
			handleSubscribe();
		} else {
			response.code = HTTP_STATUS_BAD_REQUEST;
		}
		return true;
	}

	return false;
}

/*
REQUEST:

	<?xml version="1.0" encoding="utf-8"?>
	<s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/">
		<s:Body>
		<u:GetSearchCapabilities xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1" />
		</s:Body>
	</s:Envelope>

RESPONSE:

	<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
		<s:Body>
			<u:actionNameResponse xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1">
				<SearchCaps>dc:creator,dc:date,dc:title,upnp:album,upnp:actor,upnp:artist,upnp:class,upnp:genre,@refID</SearchCaps>
			</u:actionNameResponse>
		</s:Body>
	</s:Envelope>
*/

} // namespace UPnP
