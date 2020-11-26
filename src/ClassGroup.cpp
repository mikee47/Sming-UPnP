/**
 * ClassGroup.cpp
 *
 * Copyright 2020 mikee47 <mike@sillyhouse.net>
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

#include "include/Network/UPnP/ClassGroup.h"

namespace UPnP
{
const ObjectClass* ClassGroup::find(const String& type, uint8_t version) const
{
	for(unsigned i = 0; i < classes.length(); ++i) {
		auto cls = classes.data()[i];
		if(cls->typeIs(type, version)) {
			return cls;
		}
	}

	return nullptr;
}

int ClassGroup::List::add(const FlashString& domain, const ObjectClass::List& classes)
{
	for(unsigned i = 0; i < count(); ++i) {
		auto& group = elementAt(i);
		if(&group.classes == &classes) {
			return i;
		}
	}

	if(Vector<ClassGroup>::add({domain, classes})) {
		return count() - 1;
	} else {
		return -1;
	}
}

const ObjectClass* ClassGroup::List::find(const Urn& objectType) const
{
	for(unsigned i = 0; i < count(); ++i) {
		auto& group = elementAt(i);
		if(group.domain != objectType.domain) {
			continue;
		}

		auto cls = group.find(objectType.type, objectType.version);
		if(cls != nullptr) {
			return cls;
		}
	}

	return nullptr;
}

} // namespace UPnP
