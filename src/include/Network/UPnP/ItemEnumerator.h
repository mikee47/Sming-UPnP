/**
 * ItemEnumerator.h
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

#include "Enumerator.h"
#include "Item.h"

namespace UPnP
{
class Item;

class ItemEnumerator : public Enumerator<Item, ItemEnumerator>
{
public:
	ItemEnumerator(Item* head) : head_(head), current_(head)
	{
	}

	/**
	 * @brief Make a copy of this enumerator
	 * @note Each copy maintains position independently
	 */
	ItemEnumerator* clone() override
	{
		return new ItemEnumerator(*this);
	}

	/**
	 * @brief Reset enumerator to start of list
	 */
	void reset() override
	{
		current_ = head_;
	}

	Item* current() override
	{
		return current_;
	}

	/**
	 * @brief Move to next item in list
	 * @retval Item* the item, nullptr if at end
	 */
	Item* next() override
	{
		if(current_ != nullptr) {
			current_ = current_->next();
		}
		return current_;
	}

private:
	Item* head_;
	Item* current_{nullptr};
};

} // namespace UPnP
