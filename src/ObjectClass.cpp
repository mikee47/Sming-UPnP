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
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/Network/UPnP/ObjectClass.h"
#include "include/Network/UPnP/Object.h"

namespace UPnP
{
Urn ObjectClass::objectType() const
{
	return Urn(kind(), nullptr, domain(), type(), version());
}

bool ObjectClass::operator==(const ObjectClass& other) const
{
	if(this == &other) {
		return true;
	}

	if(kind() != other.kind() || version() != other.version()) {
		return false;
	}

	if(domain_ != other.domain_ && domain() != other.domain()) {
		return false;
	}

	return type_ == other.type_ || type() == other.type();
}

bool ObjectClass::typeIs(const Urn& objectType) const
{
	return kind() == objectType.kind && version() == objectType.version && domain() == objectType.domain &&
		   type() == objectType.type;
}

bool ObjectClass::typeIs(const String& type, uint8_t version) const
{
	return this->version() == version && this->type() == type;
}

} // namespace UPnP
