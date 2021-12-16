/****
 * ObjectList.h
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
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include "LinkedItemList.h"
#include "ObjectClass.h"

namespace UPnP
{
/**
 * @brief Class template for singly-linked list of objects
 * @note We don't own the objects, just keep references to them
 */
template <typename ObjectType> class ObjectList : public LinkedItemList
{
public:
	ObjectType* head()
	{
		return reinterpret_cast<ObjectType*>(LinkedItemList::head());
	}

	const ObjectType* head() const
	{
		return reinterpret_cast<const ObjectType*>(LinkedItemList::head());
	}

	typename ObjectType::Iterator begin()
	{
		return this->head();
	}

	typename ObjectType::Iterator end()
	{
		return nullptr;
	}

	bool isEmpty() const
	{
		return head() == nullptr;
	}

	size_t count() const
	{
		size_t n{0};
		for(auto it = begin(); it != end(); it++) {
			++n;
		}
		return n;
	}

	/**
	 * @brief Search list for matching entry
	 * @tparam Urn or String
	 * @retval ObjectType* Located definition or nullptr if not found
	 */
	template <typename T> ObjectType* find(const T& objectType)
	{
		for(auto it = begin(); it != end(); it++) {
			if(it->typeIs(objectType)) {
				return &(*it);
			}
		}

		return nullptr;
	}

	/**
	 * @brief Search list for matching entry given its class
	 * @objectClass Class information for object
	 * @retval ObjectType* Located definition or nullptr if not found
	 */
	ObjectType* find(const ObjectClass& objectClass)
	{
		return this->find(objectClass.objectType());
	}
};

/**
 * @brief Class template for singly-linked list of objects
 * @note We own the objects so are responsible for destroying them when removed
 */
template <typename ObjectType> class OwnedObjectList : public ObjectList<ObjectType>
{
public:
	bool remove(ObjectType* object)
	{
		bool res = LinkedItemList::remove(object);
		delete object;
		return res;
	}

	void clear()
	{
		while(remove(this->head())) {
			//
		}
	}
};

} // namespace UPnP
