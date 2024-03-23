/****
 * LinkedItemList.h
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

#include "LinkedItem.h"

namespace UPnP
{
/**
 * @brief Singly-linked list of items
 * @note We don't own the items, just keep references to them
 */
class LinkedItemList
{
public:
	bool add(LinkedItem* item);

	bool add(const LinkedItem* item)
	{
		return add(const_cast<LinkedItem*>(item));
	}

	bool remove(LinkedItem* item);

	void clear()
	{
		head_ = nullptr;
	}

	LinkedItem* head()
	{
		return head_;
	}

	const LinkedItem* head() const
	{
		return head_;
	}

	LinkedItem* find(LinkedItem* item)
	{
		auto p = head_;
		while(p != nullptr && p != item) {
			p = p->getNext();
		}
		return p;
	}

	const LinkedItem* find(LinkedItem* item) const
	{
		return const_cast<LinkedItemList*>(this)->find(item);
	}

	bool contains(LinkedItem* item) const
	{
		return find(item) != nullptr;
	}

private:
	LinkedItem* head_{nullptr};
};

} // namespace UPnP
