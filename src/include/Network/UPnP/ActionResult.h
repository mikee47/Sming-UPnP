/**
 * ActionResult.h
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

#pragma once

#include <Network/Http/HttpConnection.h>
#include "Envelope.h"

namespace UPnP
{
/**
 * @brief All action results inherit from this class.
 * We keep it as basic as possible to avoid name conflicts
 */
class ActionResult
{
public:
	ActionResult(Envelope& envelope) : envelope(envelope)
	{
	}

	template <typename T> T getArg(const FlashString& name)
	{
		T value;
		envelope.getArg(name, value);
		return value;
	}

	template <typename T> void setArg(const FlashString& name, const T& value)
	{
		envelope.addArg(name, value);
	}

	Envelope::Fault fault() const
	{
		return envelope.fault();
	}

private:
	Envelope& envelope;
};

using ActionRequest = ActionResult;

} // namespace UPnP
