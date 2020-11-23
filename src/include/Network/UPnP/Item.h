/**
 * Item.h
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

#include <RapidXML.h>

namespace UPnP
{
class Item;
class ItemEnumerator;

/**
 * @brief When building descriptions this qualifies what information is required
 */
enum class DescType {
	header,   ///< Description template with main document element
	embedded, ///< Details for embedded device or service
	content,  ///< Full details for this device or service
};

class Item
{
public:
	virtual ~Item()
	{
	}

	virtual XML::Node* getDescription(XML::Document& doc, DescType descType) const
	{
		return nullptr;
	}

	virtual ItemEnumerator* getList(unsigned index, String& name)
	{
		return nullptr;
	}

	virtual Item* next() const
	{
		return nullptr;
	}

	bool operator==(const Item& other) const
	{
		return this == &other;
	}
};

} // namespace UPnP
