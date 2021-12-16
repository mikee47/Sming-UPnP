/****
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

	struct Description {
		CString controlURL;
		CString eventSubURL;
		CString serviceId;
	};

	ServiceControl() = delete;
	ServiceControl(const ServiceControl&) = delete;

	ServiceControl(DeviceControl& device) : Service(reinterpret_cast<Device&>(device))
	{
	}

	/**
	 * @brief Get the root device
	 */
	DeviceControl& root()
	{
		return reinterpret_cast<DeviceControl&>(Service::root());
	}

	const DeviceControl& root() const
	{
		return const_cast<ServiceControl*>(this)->root();
	}

	String getField(Field desc) const override;

	ServiceControl* getNext() const
	{
		return reinterpret_cast<ServiceControl*>(next());
	}

	bool sendRequest(HttpRequest* request) const override;

	Error handleAction(ActionRequest& req) override
	{
		return Error::ActionNotImplemented;
	}

	/**
	 * @brief Called during initialisation to configure this object
	 */
	bool configure(const XML::Node* service);

	DeviceControl& device() const
	{
		return reinterpret_cast<DeviceControl&>(Service::device());
	}

	/**
	 * @brief Get service description
	 */
	const Description& description()
	{
		return description_;
	}

private:
	Description description_;
};

} // namespace UPnP
