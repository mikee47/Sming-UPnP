/**
 * ObjectClass.cpp
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
 * You should have received a copy of the GNU General Public License along with Sming UPnP.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/Network/UPnP/ObjectClass.h"

namespace UPnP
{
Urn ObjectClass::objectType() const
{
	return Urn(kind(), nullptr, String(group.domain), String(type), version());
}

bool ObjectClass::operator==(const ObjectClass& other) const
{
	if(this == &other) {
		return true;
	}

	if(kind() != other.kind() || version() != other.version()) {
		return false;
	}

	if(&group.domain != &other.group.domain && group.domain != other.group.domain) {
		return false;
	}

	return &type == &other.type || type == other.type;
}

bool ObjectClass::typeIs(const Urn& objectType) const
{
	return kind() == objectType.kind && version() == objectType.version && group.domain == objectType.domain &&
		   type == objectType.type;
}

bool ObjectClass::typeIs(const String& type, uint8_t version) const
{
	return this->version() == version && this->type == type;
}

const ObjectClass* ClassGroup::find(const Urn& objectType) const
{
	for(unsigned i = 0; i < classes.length(); ++i) {
		auto cls = classes.data()[i];
		//		debug_i("%s", toString(cls->objectType()).c_str());
		if(cls->typeIs(objectType)) {
			return cls;
		}
	}

	return nullptr;
}

const ObjectClass* ClassGroup::find(const String& type, uint8_t version) const
{
	for(unsigned i = 0; i < classes.length(); ++i) {
		auto cls = classes.data()[i];
		//		debug_i("%s", toString(cls->objectType()).c_str());
		if(cls->typeIs(type, version)) {
			return cls;
		}
	}

	return nullptr;
}

} // namespace UPnP
