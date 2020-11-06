/**
 * ControlPoint.h
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
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
 * You should have received a copy of the GNU General Public License along with FlashString.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include "Object.h"
#include "ObjectList.h"
#include <Network/SSDP/Message.h>
#include <Network/HttpClient.h>
#include "Urn.h"
#include "Constants.h"

namespace UPnP
{
class ControlPoint : public ObjectTemplate<ControlPoint>
{
public:
	using DescriptionCallback = Delegate<void(HttpConnection& connection, XML::Document& description)>;

	ControlPoint(size_t maxDescriptionSize = 2048) : maxDescriptionSize(maxDescriptionSize)
	{
	}

	/**
	 * @brief Called by framework to handle an incoming SSDP message
	 * @param msg
	 */
	virtual void onNotify(BasicMessage& msg) = 0;

	/* Object */

	RootDevice* getRoot() override
	{
		return nullptr;
	}

	void search(const SearchFilter& filter) override
	{
	}

	bool formatMessage(Message& msg, MessageSpec& ms) override
	{
		return true;
	}

	bool onHttpRequest(HttpServerConnection& connection) override
	{
		return false;
	}

	/**
	 * @brief Send a request
	 * @param request Completed request object: leave response stream unassigned, will be set later
	 * @retval bool true on success, false if queue is full
	 */
	bool sendRequest(HttpRequest* request);

	/**
	 * @brief Send a request for description document
	 * @param request Completed request (response stream unassigned)
	 * @param callback To be invoked with requested document
	 * @retval bool true on success, false if queue is full
	 */
	bool sendDescriptionRequest(HttpRequest* request, DescriptionCallback callback);

	/**
	 * @brief Send a request for description document
	 * @param request Description URL
	 * @param callback To be invoked with requested document
	 * @retval bool true on success, false if queue is full
	 */
	bool requestDescription(const String& url, DescriptionCallback callback);

private:
	void processDescriptionResponse(HttpConnection& connection, DescriptionCallback callback);

	static HttpClient http;
	size_t maxDescriptionSize; // <<< Maximum size of XML description that can be processed
};

using ControlPointList = ObjectList<ControlPoint>;

} // namespace UPnP
