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
#include "Urn.h"
#include "Constants.h"
#include "DeviceControl.h"
#include "ServiceControl.h"
#include <memory>

namespace UPnP
{
class ControlPoint : public ObjectTemplate<ControlPoint>
{
public:
	using DescriptionCallback = Delegate<void(HttpConnection& connection, XML::Document& description)>;

	/**
	 * @brief Callback invoked when device has been located
	 * @param device Must be deleted when finished with
	 */
	using DeviceControlCallback = Delegate<void(DeviceControl* device)>;

	/**
	 * @brief Callback invoked when service has been located
	 * @param device Must be deleted when finished with
	 */
	using ServiceControlCallback = Delegate<void(DeviceControl* device, ServiceControl* service)>;

	ControlPoint(size_t maxDescriptionSize = 2048) : maxDescriptionSize(maxDescriptionSize)
	{
		controlPoints.add(this);
	}

	~ControlPoint()
	{
		controlPoints.remove(this);
	}

	/**
	 * @brief Searches for UPnP device or service and fetches its description
	 * @param urn unique identifier of the service or device to find
	 * @param callback Invoked with device description document
	 * @retval bool true on success, false if request queue is full
	 */
	bool beginSearch(const Urn& urn, DescriptionCallback callback)
	{
		return submitSearch(new Search(urn, callback));
	}

	/**
	 * @brief Searches for UPnP device
	 * @param cls Device class object
	 * @param callback Invoked with constructed control object
	 * @retval bool true on success, false if request queue is full
	 */
	bool beginSearch(const DeviceClass& cls, DeviceControlCallback callback)
	{
		return submitSearch(new Search(cls, callback));
	}

	/**
	 * @brief Searches for UPnP service
	 * @param cls Service class object
	 * @param callback Invoked with constructed control object
	 * @retval bool true on success, false if request queue is full
	 */
	bool beginSearch(const ServiceClass& cls, ServiceControlCallback callback)
	{
		return submitSearch(new Search(cls, callback));
	}

	template <typename Device> bool beginSearch(Delegate<void(Device*)> callback)
	{
		// TODO: Keep owned list of these
		// TODO: Create OwnedObjectList<>
		return submitSearch(new Search(
			getDeviceClass<typename Device::Class>(),
			DeviceControlCallback([callback](DeviceControl* device) { callback(reinterpret_cast<Device*>(device)); })));
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

	RootDevice* getRoot() override
	{
		return nullptr;
	}

	void search(const SearchFilter& filter) override
	{
	}

	bool formatMessage(Message& msg, MessageSpec& ms) override;

	bool onHttpRequest(HttpServerConnection& connection) override
	{
		return false;
	}

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
	bool requestDescription(const String& url, DescriptionCallback callback);

	/**
	 * @brief Called via SSDP when incoming message received
	 */
	static void onSsdpMessage(BasicMessage& msg);

private:
	using List = ObjectList<ControlPoint>;

	struct Search {
		struct Device {
			const DeviceClass* cls;
			DeviceControlCallback callback;
		};
		struct Service {
			const ServiceClass* cls;
			ServiceControlCallback callback;
		};
		struct Desc {
			UPnP::Urn urn;
			DescriptionCallback callback;
		};
		enum class Kind {
			desc,	///< Fetch description for any matching urn
			device,  ///< Searching for pre-defined device class
			service, ///< Searching for pre-defined service class
		};

		Search(const Urn& urn, DescriptionCallback callback) : kind(Kind::desc)
		{
			desc.urn = urn;
			desc.callback = callback;
		}

		Search(const DeviceClass& cls, DeviceControlCallback callback) : kind(Kind::device)
		{
			device.cls = &cls;
			device.callback = callback;
		}

		Search(const ServiceClass& cls, ServiceControlCallback callback) : kind(Kind::service)
		{
			service.cls = &cls;
			service.callback = callback;
		}

		Urn getUrn() const
		{
			switch(kind) {
			case Kind::desc:
				return desc.urn;
			case Kind::device:
				return device.cls->getUrn();
			case Kind::service:
				return service.cls->getUrn();
			default:
				assert(false);
				return Urn{};
			}
		}

		String toString(Search::Kind kind) const
		{
			switch(kind) {
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
			s += ::toString(getUrn());
			s += '}';
			return s;
		}

		Kind kind;
		Device device;
		Service service;
		Desc desc;
	};

	template <typename Class> const DeviceClass& getDeviceClass()
	{
		const DeviceClass* c = deviceClasses.head();
		while(c != nullptr) {
			if(c->equals<Class>()) {
				return *c;
			}

			c = c->getNext();
		}

		c = new Class;
		deviceClasses.add(c);
		return *c;
	}

	bool submitSearch(Search* search);
	bool processDescriptionResponse(HttpConnection& connection, XML::Document& description);

	static List controlPoints;
	static DeviceClass::OwnedList deviceClasses;
	static HttpClient http;
	size_t maxDescriptionSize; // <<< Maximum size of XML description that can be processed
	CStringArray uniqueServiceNames;
	std::unique_ptr<Search> activeSearch;
};

} // namespace UPnP
