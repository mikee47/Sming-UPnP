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

#pragma once

#include "ActionResponse.h"

namespace UPnP
{
class ActionRequest : public ActionResponse
{
public:
	ActionRequest(Envelope& envelope, Stream* stream) : ActionResponse(envelope, stream)
	{
	}

private:
	friend ActionResponse;
};

class ActionRequestControl : public ActionResponse
{
public:
	using Callback = Delegate<void(ActionResponse response)>;

	ActionRequestControl(const Service& service, const String& actionName)
		: ActionResponse(envelope, nullptr), envelope(service)
	{
		envelope.createRequest(actionName);
	}

	bool send(const Callback& callback);

private:
	Envelope envelope;
};

} // namespace UPnP
