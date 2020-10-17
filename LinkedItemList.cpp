/**
 * LinkedItemList.cpp
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

#include <Network/UPnP/LinkedItemList.h>

namespace UPnP
{
bool LinkedItemList::add(LinkedItem* item)
{
	if(item == nullptr) {
		return false;
	}

	LinkedItem* prev = nullptr;
	auto it = head_;
	while(it != nullptr) {
		if(it == item) {
			// Already in list
			return true;
		}
		prev = it;
		it = it->next_;
	}

	if(prev == nullptr) {
		head_ = item;
	} else {
		prev->next_ = item;
	}
	item->next_ = it;
	return true;
}

bool LinkedItemList::remove(LinkedItem* item)
{
	if(item == nullptr) {
		return false;
	}

	if(head_ == item) {
		head_ = item->next_;
		return true;
	}

	auto it = head_;
	while(it->next_ != nullptr) {
		if(it->next_ == item) {
			it->next_ = item->next_;
			break;
		}
		it = it->next_;
	}
	item->next_ = nullptr;
	return true;
}

} // namespace UPnP
