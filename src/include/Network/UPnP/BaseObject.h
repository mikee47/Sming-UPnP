/**
 * BaseObject.h
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

#include "LinkedItem.h"
#include "ObjectClass.h"
#include <WString.h>
#include <Delegate.h>
#include <Network/SSDP/MessageSpec.h>
#include <Data/Stream/DataSourceStream.h>
#include <Network/Http/HttpServerConnection.h>
#include <Network/Http/HttpRequest.h>
#include <Network/Http/HttpResponse.h>
#include <Network/SSDP/Urn.h>

namespace UPnP
{
using namespace SSDP;

class BaseObject;

struct SearchFilter {
	using Callback = Delegate<void(BaseObject* object, SearchMatch match)>;

	SearchFilter(const MessageSpec& ms, uint32_t delayMs) : ms(ms), delayMs(delayMs)
	{
	}

	const MessageSpec& ms; ///< Specification for message to be sent
	uint32_t delayMs;	  ///< Message delay
	String targetString;   ///< Full search target value
	Callback callback;	 ///< Called on a match
};

/**
 * @brief Objects which hook into the SSDP message stack
 */
class BaseObject : public LinkedItem
{
public:
	/**
	 * @brief Standard fields have been completed
	 * @note Fields can be modified typically by adding any custom fields
	 * before sending response.
	 * @param msg The message being constructed
	 * @param ms Template spec. for message
	 * @retval bool Return true to send message, false to cancel
	 */
	virtual bool formatMessage(Message& msg, MessageSpec& ms) = 0;

	/**
	 * @brief Called by framework to construct then send a message.
	 * @param msg Message to send
	 * @param ms Template spec.
	 */
	virtual void sendMessage(Message& msg, MessageSpec& ms);
};

/**
 * @brief Base class template for linked items with type casting
 */
template <typename ObjectType, typename BaseObjectType> class ObjectTemplate : public BaseObjectType
{
public:
	class Iterator : public std::iterator<std::forward_iterator_tag, ObjectType>
	{
	public:
		Iterator(ObjectType* x) : o(x)
		{
		}

		Iterator(ObjectType& x) : o(&x)
		{
		}

		Iterator(const Iterator& other) : o(other.o)
		{
		}

		Iterator& operator++()
		{
			o = o->getNext();
			return *this;
		}

		Iterator operator++(int)
		{
			Iterator tmp(*this);
			operator++();
			return tmp;
		}

		bool operator==(const Iterator& rhs) const
		{
			return o == rhs.o;
		}

		bool operator!=(const Iterator& rhs) const
		{
			return o != rhs.o;
		}

		ObjectType& operator*()
		{
			return *o;
		}

		ObjectType* operator->()
		{
			return o;
		}

		operator ObjectType*()
		{
			return o;
		}

	private:
		ObjectType* o;
	};

	ObjectType* getNext() const
	{
		return reinterpret_cast<ObjectType*>(this->next());
	}

	Iterator begin() const
	{
		return Iterator(this);
	}

	Iterator end() const
	{
		return Iterator(nullptr);
	}
};

} // namespace UPnP
