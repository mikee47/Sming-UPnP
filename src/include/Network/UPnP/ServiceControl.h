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
	using Field = Service::Field;

	class ArgList
	{
	public:
		void addInput(const String& name, const String& value)
		{
			// TODO
		}

		void addOutput(const String& name)
		{
			// TODO
		}
	};

	ServiceControl(DeviceControl& device, const ServiceClass& serviceClass) : device(device), cls(serviceClass)
	{
	}

	String getField(Field desc) const override;

	template <typename... ParamTypes> bool DispatchRequest(Delegate<void(ParamTypes...)>, ArgList args);

	void handleAction(ActionInfo& info) override
	{
	}

private:
	DeviceControl& device;
	const ServiceClass& cls;
};

} // namespace UPnP
