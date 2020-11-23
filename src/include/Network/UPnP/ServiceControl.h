/**
 * ServiceControl.h
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

#include "ServiceClass.h"
#include "Service.h"
#include <Data/CString.h>
#include <memory>

namespace UPnP
{
class DeviceControl;

class ServiceControl : public Service
{
public:
	using List = ObjectList<ServiceControl>;
	using OwnedList = OwnedObjectList<ServiceControl>;
	using Field = Service::Field;

	ServiceControl() = delete;
	ServiceControl(const ServiceControl&) = delete;

	ServiceControl(DeviceControl& device) : Service(reinterpret_cast<Device&>(device))
	{
	}

	RootDeviceControl& root() const
	{
		return reinterpret_cast<RootDeviceControl&>(Service::root());
	}

	String getField(Field desc) const override;

	virtual const ServiceClass& getClass() const = 0;

	Version version() const override
	{
		return getClass().version();
	}

	ServiceControl* getNext() const
	{
		return reinterpret_cast<ServiceControl*>(next());
	}

	bool sendRequest(ActionInfo& request, const ActionInfo::Callback& callback);

	void handleAction(ActionInfo& info) override
	{
	}

	bool configure(const XML::Node* service);

	DeviceControl& device() const
	{
		return reinterpret_cast<DeviceControl&>(Service::device());
	}

private:
	struct Description {
		CString controlURL;
		CString eventSubURL;
		CString serviceId;
	};
	Description description;
};

} // namespace UPnP
