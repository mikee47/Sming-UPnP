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

bool ControlPoint::beginSearch(const UPnP::Urn& urn, DescriptionCallback callback)
{
	if(searchUrn) {
		debug_e("Search already in progress");
		return false;
	}

	if(!initialize()) {
		return false;
	}

	searchUrn = urn;
	searchCallback = callback;

	auto message = new SSDP::MessageSpec(SSDP::MessageType::msearch, SSDP::SearchTarget::root, this);
	message->setRepeat(2);
	SSDP::server.messageQueue.add(message, 0);

	return true;
}

bool ControlPoint::formatMessage(SSDP::Message& message, SSDP::MessageSpec& ms)
{
	// Override the search target
	message["ST"] = searchUrn;
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
	String st = searchUrn;
	if(st != message["NT"] && st != message["ST"]) {
		return;
	}

	auto location = message[HTTP_HEADER_LOCATION];
	if(location == nullptr) {
		debug_d("CP: No valid Location header found.");
		return;
	}

	auto uniqueServiceName = message["USN"];
	if(uniqueServiceName == nullptr) {
		debug_d("CP: No valid USN header found.");
		return;
	}

	if(uniqueServiceNames.contains(uniqueServiceName)) {
		return; // Already found
	}

	if(requestDescription(location, searchCallback)) {
		// Request queued
		// TODO: Consider what happens if request fails to complete
		uniqueServiceNames += uniqueServiceName;
	}
}

// TODO: How to inform client of failed fetch?
void ControlPoint::processDescriptionResponse(HttpConnection& connection, DescriptionCallback callback)
{
	debug_i("Received description");
	auto response = connection.getResponse();
	if(response->stream == nullptr) {
		debug_e("No body");
		return;
	}

	if(!callback) {
		debug_w("ControlPoint: No description callback provided");
		return;
	}

	String content;
	response->stream->moveString(content);
	XML::Document doc;
	if(!XML::deserialize(doc, content)) {
		debug_w("Failed to deserialize XML");
		return;
	}

	callback(connection, doc);
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
		} else {
			processDescriptionResponse(connection, callback);
		}
		return 0;
	});

	return sendRequest(request);
}
} // namespace UPnP
