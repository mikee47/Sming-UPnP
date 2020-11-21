/**
 * RootDeviceControl.h
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
 * You should have received a copy of the GNU General Public License along with Sming UPnP.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include "DeviceControl.h"

namespace UPnP
{
class ControlPoint;

class RootDeviceControl : public DeviceControl
{
public:
	using List = ObjectList<RootDeviceControl>;
	using OwnedList = OwnedObjectList<RootDeviceControl>;

	void search(const SearchFilter& filter) override
	{
	}

	bool formatMessage(Message& msg, MessageSpec& ms) override
	{
		return false;
	}

	bool sendRequest(ActionInfo& request, const ActionInfo::Callback& callback);

	bool configure(ControlPoint& controlPoint, const Url& location, XML::Document& description);

	String getField(Field desc) const;

	String baseURL() const
	{
		return rootConfig->baseUrl.c_str();
	}

	ControlPoint& controlPoint() const
	{
		return rootConfig->controlPoint;
	}
};

} // namespace UPnP
