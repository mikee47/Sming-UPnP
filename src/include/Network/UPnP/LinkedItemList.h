/**
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
 * You should have received a copy of the GNU General Public License along with Sming UPnP.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include "LinkedItem.h"

namespace UPnP
{
/**
 * @brief Singly-linked list of objects
 */
class LinkedItemList
{
public:
	bool add(LinkedItem* item);

	bool remove(LinkedItem* item);

	LinkedItem* head()
	{
		return head_;
	}

private:
	LinkedItem* head_{nullptr};
};

} // namespace UPnP
