/**
 * ControlPoint.h
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

#include "Object.h"
#include "ObjectList.h"
#include <Network/SSDP/Message.h>
#include <Network/HttpClient.h>
#include "Constants.h"
#include "RootDeviceControl.h"
#include "Search.h"
#include <memory>

namespace UPnP
{
class ControlPoint : public ObjectTemplate<ControlPoint>
{
public:
	/**
	 * @brief Constructor
	 * @param maxResponseSize Limits size of stream used to receive HTTP responses
	 */
	ControlPoint(size_t maxResponseSize = 2048) : maxResponseSize(maxResponseSize)
	{
		controlPoints.add(this);
	}

	~ControlPoint()
	{
		controlPoints.remove(this);
	}

	/**
	 * @brief Cancel any outstanding search and reset the list of known unique service names
	 */
	void reset()
	{
		cancelSearch();
		uniqueServiceNames.clear();
	}

	/**
	 * @brief Perform a reset and destroy all created devices
	 */
	void clear()
	{
		reset();
		rootDevices.clear();
	}

	/**
	 * @brief Searches for UPnP device or service and returns SSDP response messages
	 * @param urn unique identifier of the service or device to find
	 * @param callback Invoked with SSDP response message
	 * @retval bool true on success, false if request queue is full
	 */
	bool beginSearch(const Urn& urn, SsdpSearch::Callback callback)
	{
		return submitSearch(new SsdpSearch(urn, callback));
	}

	/**
	 * @brief Searches for UPnP device or service and fetches its description
	 * @param urn unique identifier of the service or device to find
	 * @param callback Invoked with device description document
	 * @retval bool true on success, false if request queue is full
	 */
	bool beginSearch(const Urn& urn, DescriptionSearch::Callback callback)
	{
		return submitSearch(new DescriptionSearch(urn, callback));
	}

	/**
	 * @brief Searches for UPnP device
	 * @param cls Device class object
	 * @param callback Invoked with constructed control object
	 * @retval bool true on success, false if request queue is full
	 */
	bool beginSearch(const DeviceClass& cls, DeviceSearch::Callback callback)
	{
		return submitSearch(new DeviceSearch(cls, callback));
	}

	/**
	 * @brief Searches for UPnP service
	 * @param cls Service class object
	 * @param callback Invoked with constructed control object
	 * @retval bool true on success, false if request queue is full
	 */
	bool beginSearch(const ServiceClass& cls, ServiceSearch::Callback callback)
	{
		return submitSearch(new ServiceSearch(cls, callback));
	}

	template <typename Device> bool beginSearch(Delegate<bool(Device&)> callback)
	{
		return beginSearch(Device(nullptr).getClass(),
						   [callback](DeviceControl& device) { return callback(reinterpret_cast<Device&>(device)); });
	}

	/**
	 * @brief Determine if there's an active search in progress
	 */
	bool isSearchActive() const
	{
		return bool(activeSearch);
	}

	/**
	 * @brief Cancel any active search operation
	 * @retval bool true if a search was active, false if there was no active search
	 * @todo Set timeout on search operation and call this automatically
	 * Need to inform application though - perhaps a generic callback on the class?
	 */
	bool cancelSearch();

	/**
	 * @brief Called by framework to handle an incoming SSDP message
	 * @param msg
	 */
	virtual void onNotify(BasicMessage& msg);

	/* Object */

	Version version() const
	{
		return 0;
	}

	void search(const SearchFilter& filter) override
	{
	}

	bool formatMessage(Message& msg, MessageSpec& ms) override;

	bool onHttpRequest(HttpServerConnection& connection) override
	{
		return false;
	}

	bool sendRequest(ActionInfo& request, const ActionInfo::Callback& callback);

	/**
	 * @brief Send a request
	 * @param request Completed request object: leave response stream unassigned, will be set later
	 * @retval bool true on success, false if queue is full
	 */
	bool sendRequest(HttpRequest* request);

	/**
	 * @brief Send a request for description document
	 * @param request Description URL
	 * @param callback To be invoked with requested document
	 * @retval bool true on success, false if queue is full
	 */
	bool requestDescription(const String& url, DescriptionSearch::Callback callback);

	/**
	 * @brief Called via SSDP when incoming message received
	 */
	static void onSsdpMessage(BasicMessage& msg);

	static const ObjectClass* findClass(const Urn& objectType);

	template <typename Class> static const Class* findClass()
	{
		const Class* cls = findClass(Class().objectType());
		return reinterpret_cast<const Class*>(cls);
	}

	static const ServiceClass* findServiceClass(const Urn& urn)
	{
		return (urn.kind == Urn::Kind::service) ? reinterpret_cast<const ServiceClass*>(findClass(urn)) : nullptr;
	}

	static const DeviceClass* findDeviceClass(const Urn& urn)
	{
		return (urn.kind == Urn::Kind::device) ? reinterpret_cast<const DeviceClass*>(findClass(urn)) : nullptr;
	}

	static void registerClasses(const ClassGroup& group)
	{
		if(!objectClasses.contains(&group)) {
			objectClasses.add(&group);
		}
	}

private:
	using List = ObjectList<ControlPoint>;

	bool submitSearch(Search* search);
	static bool processDescriptionResponse(HttpConnection& connection, String& buffer, XML::Document& description);

	static List controlPoints;
	static ClassGroup::List objectClasses;
	RootDeviceControl::OwnedList rootDevices;
	static HttpClient http;
	size_t maxResponseSize; // <<< Maximum size of XML description that can be processed
	CStringArray uniqueServiceNames;
	std::unique_ptr<Search> activeSearch;
};

} // namespace UPnP
