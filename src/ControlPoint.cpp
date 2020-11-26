/**
 * ControlPoint.cpp
 *
 * Copyright 2020 mikee47 <mike@sillyhouse.net>
 * Copyright 2020 slaff <slaff@attachix.com>
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

#include "include/Network/UPnP/ControlPoint.h"
#include "include/Network/UPnP/Envelope.h"
#include <Network/SSDP/Server.h>
#include "DescriptionParser.h"
#include "main.h"
#include <Data/Stream/MemoryDataStream.h>

namespace UPnP
{
ControlPoint::List ControlPoint::controlPoints;
HttpClient ControlPoint::http;
ClassGroup::List ControlPoint::objectClasses;

const ObjectClass* ControlPoint::findClass(const Urn& objectType)
{
	const ObjectClass* cls = objectClasses.find(objectType);
	if(cls == nullptr) {
		debug_w("Class '%s' not registered", toString(objectType).c_str());
	} else {
		debug_i("Found %s class '%s'", toString(cls->kind()).c_str(), String(cls->type()).c_str());
	}

	return cls;
}

bool ControlPoint::submitSearch(Search* search)
{
	if(bool(activeSearch)) {
		debug_e("Search already in progress");
		delete search;
		return false;
	}

	if(!bool(*search)) {
		debug_e("Invalid search");
		delete search;
		return false;
	}

	if(!initialize()) {
		delete search;
		return false;
	}

	activeSearch.reset(search);

	auto message = new SSDP::MessageSpec(SSDP::MessageType::msearch, SSDP::SearchTarget::type, this);
	message->setRepeat(2);
	SSDP::server.messageQueue.add(message, 0);
	debug_i("Searching for %s", search->toString().c_str());

	return true;
}

bool ControlPoint::cancelSearch()
{
	if(!bool(activeSearch)) {
		return false;
	}

	debug_i("Cancelling search for %s", activeSearch->toString().c_str());

	SSDP::server.messageQueue.remove(this);
	activeSearch.reset();
	return true;
}

bool ControlPoint::formatMessage(SSDP::Message& message, SSDP::MessageSpec& ms)
{
	// Override the search target
	if(!bool(activeSearch)) {
		assert(false);
		return false;
	}
	message["ST"] = activeSearch->urn;
	if(UPNP_VERSION_IS(2.0)) {
		message[F("CPFN.UPNP.ORG")] = F("Sming ControlPoint");
	}
	return true;
}

void ControlPoint::onSsdpMessage(BasicMessage& msg)
{
	for(auto cp = controlPoints.head(); cp != nullptr; cp = cp->getNext()) {
		cp->onNotify(msg);
	}
}

void ControlPoint::onNotify(SSDP::BasicMessage& message)
{
	if(!bool(activeSearch)) {
		return;
	}

	auto st = activeSearch->urn;
	if(st != message["NT"] && st != message["ST"]) {
		return;
	}

	auto location = message[HTTP_HEADER_LOCATION];
	if(location == nullptr) {
		debug_w("CP: No valid Location header found.");
		return;
	}

	String uniqueServiceName = message["USN"];
	if(!uniqueServiceName) {
		debug_w("CP: No valid USN header found.");
		return;
	}

	if(uniqueServiceNames.contains(uniqueServiceName)) {
		return; // Already found
	}

	debug_i("Found match for %s", activeSearch->toString().c_str());
	debug_i("  location: %s", location);
	debug_i("  usn: %s", uniqueServiceName.c_str());

	if(activeSearch->kind == Search::Kind::ssdp) {
		auto& search = *reinterpret_cast<SsdpSearch*>(activeSearch.get());
		search.callback(message);
		return;
	}

	debug_d("Fetching description from URL: '%s'", location);
	auto request = new HttpRequest(location);

	if(activeSearch->kind == Search::Kind::desc) {
		request->onRequestComplete([this, uniqueServiceName](HttpConnection& connection, bool success) -> int {
			if(!bool(activeSearch)) {
				// Looks like search was cancelled
				return 0;
			}

			if(uniqueServiceNames.contains(uniqueServiceName)) {
				return 0;
			}
			if(success) {
				// Don't retry
				uniqueServiceNames += uniqueServiceName;
			}

			String content;
			XML::Document description;
			bool ok = processDescriptionResponse(connection, content, description);

			auto& search = *reinterpret_cast<DescriptionSearch*>(activeSearch.get());
			search.callback(connection, ok ? &description : nullptr);

			return 0;
		});
		return;
	}

	request->setResponseStream(new DescriptionParser(*this, location));

	request->onRequestComplete([this, uniqueServiceName](HttpConnection& connection, bool success) -> int {
		if(!bool(activeSearch)) {
			// Looks like search was cancelled
			return 0;
		}

		if(uniqueServiceNames.contains(uniqueServiceName)) {
			return 0;
		}
		if(success) {
			// Don't retry
			uniqueServiceNames += uniqueServiceName;
		}

		auto response = connection.getResponse();

		if(!response->isSuccess()) {
			debug_e("[UPnP] failed: %s", toString(response->code).c_str());
			return 0;
		}

		assert(response->stream != nullptr);
		auto parser = reinterpret_cast<DescriptionParser*>(response->stream);

		switch(activeSearch->kind) {
		case Search::Kind::device: {
			auto& search = *reinterpret_cast<DeviceSearch*>(activeSearch.get());

			auto device = parser->rootDevice;
			parser->rootDevice = nullptr;

			rootDevices.add(device);

			/******
			 * Deferring the callback allows the stack to unwind first.
			 * However, we get `HPE_CLOSED_CONNECTION` error when attempting to send any HTTP
			 * requests from that callback, e.g. sending action request.
			 *
			 * Invoking the callback directly and it works.
			 *
			 * Why is that? How to fix it?
			 */
			//			auto callback = search.callback;
			//			System.queueCallback([this, callback, device]() {
			//				if(!callback(*device)) {
			//					rootDevices.remove(device);
			//				}
			//			});
			if(!search.callback(*device)) {
				rootDevices.remove(device);
			}
			/*****/

			break;
		}

		case Search::Kind::service: {
			auto& search = *reinterpret_cast<ServiceSearch*>(activeSearch.get());
			uniqueServiceNames += uniqueServiceName;

			Usn usn(uniqueServiceName);
			if(!usn) {
				debug_e("[UPnP] Invalid USN: %s", uniqueServiceName.c_str());
				break;
			}

			auto device = parser->rootDevice;
			parser->rootDevice = nullptr;

			ServiceControl* service = device->getService(search.cls);
			if(service == nullptr) {
				delete device;
				break;
			}

			rootDevices.add(device);

			/***** See above */
			//			auto callback = search.callback;
			//			System.queueCallback([this, callback, device, service]() {
			//				if(!callback(*device, *service)) {
			//					rootDevices.remove(device);
			//				}
			//			});

			if(!search.callback(*device, *service)) {
				rootDevices.remove(device);
			}
			/*****/

			break;
		}

		default:
			assert(false);
		}

		return 0;
	});

	// If request queue is full we can try again later
	sendRequest(request);
}

bool ControlPoint::processDescriptionResponse(HttpConnection& connection, String& content, XML::Document& description)
{
	debug_i("Received description");

	auto response = connection.getResponse();

	if(!response->isSuccess()) {
		debug_e("[UPnP] failed: %s", toString(response->code).c_str());
		return false;
	}

	if(response->stream == nullptr) {
		debug_e("[UPnP] No response body");
		return false;
	}

	if(!response->stream->moveString(content)) {
		// TODO: Implement XML streaming parser
		debug_e("[UPnP] Description too big: increase maxResponseSize");
	}

	if(content.length() == 0) {
		debug_e("[UPnP] No description body");
	}

	if(!XML::deserialize(description, content)) {
		debug_w("Failed to deserialize XML");
		return false;
	}

	return true;
}

bool ControlPoint::sendRequest(Envelope& env, const Envelope::Callback& callback)
{
	auto req = new HttpRequest;
	req->setMethod(HttpMethod::POST);
	String url = env.service.device().getUrl(env.service.getField(Service::Field::controlURL));
	if(!url) {
		debug_e("[UPnP] No service endpoint defined");
		return false;
	}
	req->uri = url;
	auto stream = new MemoryDataStream;
	env.serialize(*stream, false);
	req->setBody(stream);

	String s = toString(MimeType::XML);
	s += F("; charset=\"utf-8\"");
	req->headers[HTTP_HEADER_CONTENT_TYPE] = s;
	req->headers[F("SOAPACTION")] = env.soapAction();

#if DEBUG_VERBOSE_LEVEL == DBG
	s = req->toString();
	s += env.serialize(true);
	m_nputs(s.c_str(), s.length());
	m_puts("\r\n");
#endif

	const Service* service = &env.service;
	req->onRequestComplete([service, callback](HttpConnection& client, bool successful) -> int {
		Envelope response(*service);
		String s;
#if DEBUG_VERBOSE_LEVEL == DBG
		s = client.getResponse()->toString();
		m_nputs(s.c_str(), s.length());
#endif
		s = client.getResponse()->getBody();
#if DEBUG_VERBOSE_LEVEL == DBG
		m_nputs(s.c_str(), s.length());
		m_puts("\r\n");
#endif
		response.load(std::move(s));
		callback(response);
		return 0;
	});

	return sendRequest(req);
}

bool ControlPoint::sendRequest(HttpRequest* request)
{
	// Don't create response stream until headers are in: this allows requests to be queued
	if(request != nullptr && request->getResponseStream() == nullptr) {
		request->onHeadersComplete([this](HttpConnection& client, HttpResponse& response) -> int {
			client.getRequest()->setResponseStream(new LimitedMemoryStream(maxResponseSize));
			return 0;
		});
	}

	return http.send(request);
}

bool ControlPoint::requestDescription(const String& url, DescriptionSearch::Callback callback)
{
	debug_d("Fetching description from URL: '%s'", url.c_str());
	auto request = new HttpRequest(url);

	request->onRequestComplete([callback](HttpConnection& connection, bool success) -> int {
		if(!success) {
			debug_e("[UPnP] Description fetch failed");
		}

		if(callback) {
			String content;
			XML::Document description;
			bool ok = processDescriptionResponse(connection, content, description);
			callback(connection, ok ? &description : nullptr);
		} else {
			debug_w("[UPnP]: No description callback provided");
		}

		return 0;
	});

	return sendRequest(request);
}

} // namespace UPnP
