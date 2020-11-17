/**
 * Search.h
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 * Copyright 2020 slaff <slaff@attachix.com>
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
#include "ServiceControl.h"

namespace UPnP
{
/**
 * @brief This is a helper class used by ControlPoint to manage different search types
 */
struct Search {
	enum class Kind {
		none,	///< No search active
		ssdp,	///< SSDP response
		desc,	///< Fetch description for any matching urn
		device,  ///< Searching for pre-defined device class
		service, ///< Searching for pre-defined service class
	};

	Search() = default;

	Search(const Search&) = delete;

	Search(Kind kind, const String& urn) : kind(kind), urn(urn)
	{
	}

	Search(Kind kind, const Urn& urn) : Search(kind, String(urn))
	{
	}

	virtual ~Search()
	{
	}

	String toString(Search::Kind kind) const
	{
		switch(kind) {
		case Kind::ssdp:
			return F("SSDP");
		case Kind::desc:
			return F("Description");
		case Kind::device:
			return F("Device");
		case Kind::service:
			return F("Service");
		default:
			assert(false);
			return nullptr;
		}
	}

	String toString() const
	{
		String s = toString(kind);
		s += " {";
		s += urn;
		s += '}';
		return s;
	}

	Kind kind;
	String urn;
};

struct SsdpSearch : public Search {
	using Callback = Delegate<void(SSDP::BasicMessage& message)>;

	SsdpSearch(const Urn& urn, Callback callback) : Search(Kind::ssdp, urn), callback(callback)
	{
	}

	Callback callback;
};

struct DescriptionSearch : public Search {
	using Callback = Delegate<void(HttpConnection& connection, XML::Document& description)>;

	DescriptionSearch(const Urn& urn, Callback callback) : Search(Kind::desc, urn), callback(callback)
	{
	}

	Callback callback;
};

/**
 * @brief Callback invoked when device has been located
 * @param device
 * @retval bool Return true to keep the device, false to destroy it
 * @note If callback sends out action requests then must return true
 */
struct DeviceSearch : public Search {
	using Callback = Delegate<bool(DeviceControl& device)>;

	DeviceSearch(const DeviceClass& cls, Callback callback)
		: Search(Kind::device, cls.getDeviceType()), cls(cls), callback(callback)
	{
	}

	const DeviceClass& cls;
	Callback callback;
};

/**
 * @brief Callback invoked when service has been located
 * @param device
 * @param service Requested service
 * @retval bool Return true to keep the device, false to destroy it
 * @note If callback sends out action requests then must return true
 */
struct ServiceSearch : public Search {
	using Callback = Delegate<bool(DeviceControl& device, ServiceControl& service)>;

	ServiceSearch(const ServiceClass& cls, Callback callback)
		: Search(Kind::service, cls.getServiceType()), cls(cls), callback(callback)
	{
	}

	const ServiceClass& cls;
	Callback callback;
};

} // namespace UPnP
