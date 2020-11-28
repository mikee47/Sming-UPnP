/**
 * ActionResponse.h
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

#pragma once

#include "Envelope.h"
#include <Data/Stream/MemoryDataStream.h>
#include "LinkedItemList.h"

class HttpServerConnection;

namespace UPnP
{
class ActionRequest;

/**
 * @brief Class to handle action requests and responses
 */
class ActionResponse : public LinkedItem
{
public:
	/*
	 * Stream to support deferred responses
	 */
	class Stream : public MemoryDataStream
	{
	public:
		Stream(HttpServerConnection& connection, Envelope* envelope) : connection(connection), envelope(envelope)
		{
		}

		~Stream();

		bool isFinished() override
		{
			return done && MemoryDataStream::isFinished();
		}

		MimeType getMimeType() const override
		{
			return MimeType::XML;
		}

		void complete(Error err);

		Envelope* getEnvelope() const
		{
			return envelope;
		}

	private:
		friend ActionResponse;

		HttpServerConnection& connection;
		Envelope* envelope;
		LinkedItemList responses; ///< Keep track of Response objects so we can tell them when we're destroyed
		bool done{false};
	};

	ActionResponse(const ActionResponse& response) : ActionResponse(response.envelope, response.stream)
	{
	}

	ActionResponse(const ActionRequest& request);

	~ActionResponse();

	ActionResponse(Envelope& envelope, Stream* stream) : envelope(envelope), stream(stream)
	{
		if(stream != nullptr) {
			stream->responses.add(this);
		}
	}

	template <typename T> T getArg(const FlashString& name) const
	{
		T value;
		envelope.getArg(name, value);
		return value;
	}

	template <typename T> void setArg(const FlashString& name, const T& value) const
	{
		envelope.addArg(name, value);
	}

	String actionName() const
	{
		return envelope.actionName();
	}

	Envelope::Fault fault() const
	{
		return envelope.fault();
	}

	/*
	 * @brief Complete host action by sending response
	 * @param err Response error
	 * @retval bool true if response was completed, false if connection's gone
	 */
	bool complete(Error err) const;

protected:
	Envelope& envelope;
	Stream* stream;
};

} // namespace UPnP
