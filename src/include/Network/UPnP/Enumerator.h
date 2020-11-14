/**
 * List.h
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

#include "Item.h"

namespace UPnP
{
/**
 * @brief Abstract class to enumerate items
 * @note Returned items may only be considered valid for the duration of the current
 * task call as they may be destroyed at any time.
 *
 *
 * todo: We want a generic enumerator which returns Items, uses virtual methods
 * so no upcasting required
 *
 *
 */
template <typename ItemType, class EnumeratorType> class Enumerator
{
public:
	virtual ~Enumerator()
	{
	}

	/**
	 * @brief Make a copy of this enumerator
	 * @note Each copy maintains position independently
	 */
	virtual EnumeratorType* clone() = 0;

	/**
	 * @brief Reset enumerator to start of list
	 * @note Call to `next()` will return first item
	 */
	virtual void reset() = 0;

	/**
	 * @brief Get the current item
	 * @retval Item* nullptr if before start or at end of list
	 */
	virtual ItemType* current() = 0;

	/**
	 * @brief Get next item
	 * @retval Item* nullptr if no more devices
	 */
	virtual ItemType* next() = 0;
};

} // namespace UPnP
