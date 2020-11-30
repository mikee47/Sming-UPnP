/**
 * ActionResponse.cpp
 *
 * Copyright 2020 mikee47 <mike@sillyhouse.net>
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

#include "include/Network/UPnP/ActionResponse.h"
#include "include/Network/UPnP/ActionRequest.h"
#include <Network/Http/HttpServerConnection.h>

namespace UPnP
{
ActionResponse::Stream::~Stream()
{
	debug_w("[UPnP] destroy ActionResponse::Stream");
	if(auto response = responses.head()) {
		reinterpret_cast<ActionResponse*>(response)->stream = nullptr;
		response = response->next();
	}
}

void ActionResponse::Stream::complete(Error err)
{
	assert(envelope != nullptr);
	assert(responses.head() != nullptr);

	auto& httpResponse = *connection.getResponse();

	if(envelope->contentType() == Envelope::ContentType::fault) {
		httpResponse.code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
	} else if(!!err) {
		envelope->createFault(getErrorCode(err));
		httpResponse.code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
	} else if(envelope->contentType() != Envelope::ContentType::response) {
		debug_w("[UPnP] Unhandled action: %s", envelope->actionName().c_str());
		envelope->createFault(ErrorCode::OptionalActionNotImplemented);
		httpResponse.code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
	}

	envelope->serialize(*this, false);
	done = true;

#if DEBUG_VERBOSE_LEVEL >= DBG
	String s = envelope->serialize(true);
	m_puts(s.c_str());
	m_puts("\r\n");
#endif

	delete envelope;
	envelope = nullptr;

	connection.send();
}

ActionResponse::ActionResponse(const ActionRequest& request) : ActionResponse(request.envelope, request.stream)
{
	envelope.convertToResponse();
}

ActionResponse::~ActionResponse()
{
	if(stream != nullptr) {
		stream->responses.remove(this);
	}
}

bool ActionResponse::complete(Error err) const
{
	if(stream == nullptr) {
		debug_w("[UPnP] ActionResponse::complete() stream already destroyed");
		return false;
	}
	stream->complete(err);
	return true;
}

} // namespace UPnP
