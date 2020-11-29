/**
 * ActionRequest.h
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

#include "include/Network/UPnP/ActionRequest.h"
#include "include/Network/UPnP/Device.h"
#include <Network/Http/HttpRequest.h>

namespace UPnP
{
bool ActionRequestControl::send(const Callback& callback)
{
	auto& service = envelope.service;

	auto req = new HttpRequest;
	req->setMethod(HttpMethod::POST);
	String url = service.device().getUrl(service.getField(Service::Field::controlURL));
	if(!url) {
		debug_e("[UPnP] No service endpoint defined");
		return false;
	}
	req->uri = url;
	auto stream = new MemoryDataStream;
	envelope.serialize(*stream, false);
	req->setBody(stream);

	String s = toString(MimeType::XML);
	s += F("; charset=\"utf-8\"");
	req->headers[HTTP_HEADER_CONTENT_TYPE] = s;
	req->headers[F("SOAPACTION")] = envelope.soapAction();

#if DEBUG_VERBOSE_LEVEL == DBG
	s = req->toString();
	s += envelope.serialize(true);
	m_nputs(s.c_str(), s.length());
	m_puts("\r\n");
#endif

	// Don't bother checking the response if a callback wasn't provided
	if(callback) {
		req->onRequestComplete([&service, callback](HttpConnection& client, bool successful) -> int {
			Envelope env(service);
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
			env.load(std::move(s));
			ActionResponse response(env, nullptr);
			callback(response);
			return 0;
		});
	}

	return service.sendRequest(req);
}

} // namespace UPnP
