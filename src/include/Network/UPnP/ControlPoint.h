/**
 * ControlPoint.h
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
 * You should have received a copy of the GNU General Public License along with FlashString.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include "Object.h"
#include "ObjectList.h"
#include <Network/SSDP/Message.h>
#include "Urn.h"
#include "Constants.h"

namespace UPnP
{
class ControlPoint : public ObjectTemplate<ControlPoint>
{
public:
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
};

using ControlPointList = ObjectList<ControlPoint>;

} // namespace UPnP
