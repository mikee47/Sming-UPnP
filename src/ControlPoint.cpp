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
 * You should have received a copy of the GNU General Public License along with Sming UPnP.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/Network/UPnP/ControlPoint.h"
#include <Network/SSDP/Server.h>
#include "main.h"

namespace UPnP
{
ControlPoint::List ControlPoint::controlPoints;
HttpClient ControlPoint::http;

bool ControlPoint::submitSearch(Search* search)
{
	if(bool(activeSearch)) {
		debug_e("Search already in progress");
		delete search;
		return false;
	}

	if(!initialize()) {
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
	activeSearch = nullptr;
	return true;
}

bool ControlPoint::formatMessage(SSDP::Message& message, SSDP::MessageSpec& ms)
{
	// Override the search target
	if(!bool(activeSearch)) {
		assert(false);
		return false;
	}
	message["ST"] = activeSearch->getUrn();
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

	String st = activeSearch->getUrn();
	if(st != message["NT"] && st != message["ST"]) {
		return;
	}

	auto location = message[HTTP_HEADER_LOCATION];
	if(location == nullptr) {
		debug_d("CP: No valid Location header found.");
		return;
	}

	auto usn = message["USN"];
	if(usn == nullptr) {
		debug_d("CP: No valid USN header found.");
		return;
	}

	if(uniqueServiceNames.contains(usn)) {
		return; // Already found
	}

	debug_w("Found match for %s", activeSearch->toString().c_str());
	debug_w("  location: %s", location);
	debug_w("  usn: %s", usn);

	switch(activeSearch->kind) {
	case Search::Kind::desc: {
		debug_d("Fetching description from URL: '%s'", location);
		auto request = new HttpRequest(location);

		request->onRequestComplete([this, usn](HttpConnection& connection, bool success) -> int {
			if(!success) {
				debug_e("Fetch failed");
			} else if(activeSearch == nullptr) {
				// Looks like search was cancelled
			} else if(!uniqueServiceNames.contains(usn)) {
				// Don't retry
				uniqueServiceNames += usn;
				// Process and invoke callback
				XML::Document doc;
				processDescriptionResponse(connection, doc);
				if(activeSearch->desc.callback) {
					activeSearch->desc.callback(connection, doc);
				} else {
					debug_w("[UPnP]: No description callback provided");
				}
			}
			return 0;
		});

		// If request queue is full we can try again later
		sendRequest(request);
		return;
	}

	case Search::Kind::device: {
		auto& search = activeSearch->device;
		uniqueServiceNames += usn;
		if(search.callback) {
			auto device = search.cls->createObject(location, usn);
			search.callback(device);
		} else {
			debug_w("[UPnP]: No device callback provided");
		}
		break;
	}

	case Search::Kind::service: {
		auto& search = activeSearch->service;
		uniqueServiceNames += usn;
		auto device = search.cls->deviceClass().createObject(location, usn);
		if(search.callback) {
			auto service = device->getService(*search.cls);
			search.callback(device, service);
		} else {
			debug_w("[UPnP]: No service callback provided");
		}
		break;
	}

	default:
		assert(false);
	}
}

// TODO: How to inform client of failed fetch?
bool ControlPoint::processDescriptionResponse(HttpConnection& connection, XML::Document& description)
{
	debug_i("Received description");

	auto response = connection.getResponse();
	if(response->stream == nullptr) {
		debug_e("No body");
		return false;
	}

	String content;
	if(!response->stream->moveString(content)) {
		// TODO: Implement XML streaming parser
		debug_e("Description too big for buffer: Increase maxDescriptionSize");
		return false;
	}

	if(!XML::deserialize(description, content)) {
		debug_w("Failed to deserialize XML");
		return false;
	}

	return true;
}

bool ControlPoint::sendRequest(HttpRequest* request)
{
	// Don't create response stream until headers are in: this allows requests to be queued
	if(request != nullptr && request->getResponseStream() == nullptr) {
		request->onHeadersComplete([this](HttpConnection& client, HttpResponse& response) -> int {
			client.getRequest()->setResponseStream(new LimitedMemoryStream(maxDescriptionSize));
			return 0;
		});
	}

	return http.send(request);
}

bool ControlPoint::requestDescription(const String& url, DescriptionCallback callback)
{
	debug_d("Fetching description from URL: '%s'", url.c_str());
	auto request = new HttpRequest(url);

	request->onRequestComplete([this, callback](HttpConnection& connection, bool success) -> int {
		if(!success) {
			debug_e("Fetch failed");
		} else if(callback) {
			// Process and invoke callback
			XML::Document doc;
			processDescriptionResponse(connection, doc);
			callback(connection, doc);
		} else {
			debug_w("[UPnP]: No description callback provided");
		}
		return 0;
	});

	return sendRequest(request);
}
} // namespace UPnP
