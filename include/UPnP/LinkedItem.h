/**
 * LinkedItem.h
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
 * You should have received a copy of the GNU General Public License along with FlashString.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include "Item.h"

namespace UPnP
{
class LinkedItemList;

/**
 * @brief Base class template for items in a list
 */
class LinkedItem : public Item
{
public:
	LinkedItem* next() override
	{
		return next_;
	}

	LinkedItem* getNext()
	{
		return next_;
	}

private:
	friend class LinkedItemList;
	LinkedItem* next_ = nullptr;
};

} // namespace UPnP
