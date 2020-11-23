/**
 * XmlBuffer.cpp
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
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "XmlBuffer.h"

namespace UPnP
{
#define XX(t) DEFINE_FSTR(t##Tag, #t)
TAG_MAP(XX)
#undef XX

#define XX(t) &t##Tag,
DEFINE_FSTR_VECTOR(tags, FlashString, TAG_MAP(XX))
#undef XX

XmlBuffer::Match XmlBuffer::nextTag()
{
	Match m;

	for(;;) {
		int tagPos = indexOf('<', searchPos);
		if(tagPos < 0) {
			break;
		}

		m.pos = tagPos;
		++tagPos;
		m.close = (charAt(tagPos) == '/');
		if(m.close) {
			++tagPos;
		}

		int i = indexOf('>', tagPos);
		if(i < 0) {
			break;
		}

		searchPos = i + 1;

		String tag = substring(tagPos, i);
		i = tags.indexOf(tag);
		if(i >= 0) {
			m.tag = Tag(i);
			m.tagLength = 2 + tag.length() + unsigned(m.close);
			m.found = true;
			break;
		}
	}

	return m;
}

void XmlBuffer::setStart(size_t pos)
{
	remove(0, pos);
	if(searchPos > pos) {
		searchPos -= pos;
	} else {
		searchPos = 0;
	}
}

} // namespace UPnP
