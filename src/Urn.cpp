/**
 * Urn.cpp
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

#include "include/Network/UPnP/Urn.h"

namespace UPnP
{
String Urn::toString() const
{
	String kindString = ::toString(kind);
	String s;
	s.reserve(4 + domain.length() + 1 + kindString.length() + 1 + type.length() + 1 + 1);
	s += F("urn:");
	s += domain;
	s += ':';
	s += kindString;
	s += ':';
	s += type;
	s += ':';
	s += version;
	return s;
}

} // namespace UPnP

String toString(UPnP::Urn::Kind kind)
{
	using namespace UPnP;
	switch(kind) {
#define XX(tag)                                                                                                        \
	case Urn::Kind::tag:                                                                                               \
		return F(#tag);
		UPNP_URN_KIND_MAP(XX)
#undef XX
	default:
		return nullptr;
	}
}
