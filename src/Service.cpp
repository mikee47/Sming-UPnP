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
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/Network/UPnP/Device.h"
#include "include/Network/UPnP/ItemEnumerator.h"
#include "include/Network/UPnP/DescriptionStream.h"
#include <Data/Stream/MemoryDataStream.h>
#include <Data/Stream/FlashMemoryStream.h>
#include <FlashString/Vector.hpp>
#include <RapidXML.h>
#include <Network/SSDP/Uuid.h>

namespace
{
#define XX(name, req) DEFINE_FSTR_LOCAL(fn_##name, #name);
UPNP_SERVICE_FIELD_MAP(XX);
#undef XX

#define XX(name, req) &fn_##name,
DEFINE_FSTR_VECTOR(fieldNames, FlashString, UPNP_SERVICE_FIELD_MAP(XX))
#undef XX

} // namespace

String toString(UPnP::Service::Field field)
{
	return fieldNames[unsigned(field)];
}

namespace UPnP
{
Device& Service::root()
{
	return device_.root();
}

XML::Node* Service::getDescription(XML::Document& doc, DescType descType) const
{
	switch(descType) {
	case DescType::header: {
		auto root = XML::appendNode(&doc, "scpd");
		XML::appendAttribute(root, fs_xmlns, schemas_upnp_org::service_1_0);
		return root;
	}

	case DescType::embedded: {
		String s;
		auto service = XML::appendNode(&doc, _F("service"));
		for(unsigned i = 0; i < unsigned(Field::customStart); ++i) {
			s = getField(Field(i));
			if(s) {
				XML::appendNode(service, fieldNames[i], s);
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

IDataSourceStream* Service::createDescription()
{
	return new FSTR::Stream(*getClass().service().schema);
}

String Service::getField(Field desc) const
{
	// Provide defaults for required fields
	switch(desc) {
	case Field::serviceType:
		return String(objectType());
	case Field::domain:
		return getClass().domain();
	case Field::type:
		return getClass().type();
	case Field::version:
		return String(version());

	case Field::serviceId:
		return *getClass().service().serviceId;

	case Field::SCPDURL:
		return getField(Field::type) + ".xml";

	case Field::controlURL:
		return getField(Field::type) + F("-control");

	case Field::eventSubURL:
		return getField(Field::type) + F("-event");

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
	switch(filter.ms.target()) {
	case SearchTarget::all:
		filter.callback(this, SearchMatch::type);
		break;
	case SearchTarget::type:
		if(typeIs(filter.targetString)) {
			filter.callback(this, SearchMatch::type);
		}
		break;
	default:
		assert(false);
	}
}

bool Service::formatMessage(Message& msg, MessageSpec& ms)
{
	if(ms.match() != SearchMatch::type) {
		debug_e("[UPnP] Invalid search match value");
		return false;
	}

	msg[HTTP_HEADER_SERVER] = device_.getField(Device::Field::serverId);
	msg[HTTP_HEADER_LOCATION] = device().getUrl(getField(Field::SCPDURL));

	String st = String(objectType());
	String usn = device_.getField(Device::Field::UDN);
	usn += "::";
	usn += st;

	if(msg.type == MessageType::notify) {
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
				toString(request.method).c_str(), uri.Path.c_str(), getField(Field::type).c_str());

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

#if DEBUG_VERBOSE_LEVEL >= DBG
		m_puts(req.c_str());
		m_puts("\r\n");
#endif

		Envelope env(*this);
		auto err = env.load(std::move(req));
		if(!!err) {
			return;
		}

		String actionName = env.actionName();
		err = handleAction(env);

		if(env.contentType() == Envelope::ContentType::fault) {
			response.code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
		} else if(!!err) {
			env.createFault(getErrorCode(err));
			response.code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
		} else if(env.contentType() != Envelope::ContentType::response) {
			debug_e("[UPnP] Unhandled action: %s", actionName.c_str());
			env.createFault(ErrorCode::OptionalActionNotImplemented);
			response.code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
		}

		auto stream = new MemoryDataStream;
		env.serialize(*stream, false);
		device_.sendXml(response, stream);

#if DEBUG_VERBOSE_LEVEL >= DBG
		String s = env.serialize(true);
		m_puts(s.c_str());
		m_puts("\r\n");
#endif
	};

	auto handleSubscribe = [&]() {
		Uuid uuid;
		uuid.generate();

		response.headers[HTTP_HEADER_SERVER] = device_.getField(Device::Field::serverId);
		response.headers["SID"] = String("uuid:") + String(uuid);
		response.headers[HTTP_HEADER_CONTENT_LENGTH] = "0";
		response.headers["TIMEOUT"] = "1800";
		response.code = HTTP_STATUS_OK;
	};

	if(uri.Path == device().resolvePath(getField(Field::SCPDURL))) {
		printRequest();
		if(request.method == HTTP_GET) {
			device_.sendXml(response, createDescription());
		} else {
			response.code = HTTP_STATUS_BAD_REQUEST;
		}
		return true;
	}

	if(uri.Path == device().resolvePath(getField(Field::controlURL))) {
		printRequest();
		if(request.method == HTTP_POST) {
			handleControl();
		} else {
			response.code = HTTP_STATUS_BAD_REQUEST;
		}
		return true;
	}

	if(uri.Path == device().resolvePath(getField(Field::eventSubURL))) {
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
 * Simple examples
 *
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
