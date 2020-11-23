/**
 * XmlBuffer.h
 *
 * Copyright 2020 mikee47 <mike@sillyhouse.net>
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

#include <WString.h>
#include <Data/BitSet.h>
#include <FlashString/Vector.hpp>

namespace UPnP
{
#define TAG_MAP(XX)                                                                                                    \
	XX(device)                                                                                                         \
	XX(service)                                                                                                        \
	XX(iconList)                                                                                                       \
	XX(serviceList)                                                                                                    \
	XX(deviceList)

enum class Tag {
#define XX(t) t,
	TAG_MAP(XX)
#undef XX
};

using Tags = BitSet<uint16_t, Tag>;

#define XX(t) DECLARE_FSTR(t##Tag)
TAG_MAP(XX)
#undef XX

DECLARE_FSTR_VECTOR(tags, FlashString)

/**
 * @brief Class to support simple parsing of XML data
 */
class XmlBuffer : public String
{
public:
	struct Match {
		Tag tag;
		bool found{false};
		bool close;
		size_t pos;
		size_t tagLength;

		explicit operator bool() const
		{
			return found;
		}
	};

	Match nextTag();

	void setStart(size_t pos);

public:
	size_t searchPos{0};
};

} // namespace UPnP
