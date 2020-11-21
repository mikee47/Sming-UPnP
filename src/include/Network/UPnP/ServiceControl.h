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
 * You should have received a copy of the GNU General Public License along with Sming UPnP.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include "ServiceClass.h"
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

	ServiceControl(DeviceControl& device, const ServiceClass& serviceClass) : device(device), serviceClass(serviceClass)
	{
	}

	String getField(Field desc) const override;

	const ServiceClass& getClass() const
	{
		return serviceClass;
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

private:
	DeviceControl& device;
	const ServiceClass& serviceClass;
	struct Description {
		CString controlURL;
		CString eventSubURL;
	};
	Description description;
};

} // namespace UPnP
