/**
 * Error.cpp
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

#include "include/Network/UPnP/Error.h"
#include <FlashString/Vector.hpp>

namespace
{
#define XX(tag, text) DEFINE_FSTR_LOCAL(str##tag, #tag)
UPNP_ERROR_MAP(XX)
#undef XX

#define XX(tag, text) &str##tag,
DEFINE_FSTR_VECTOR(strings, FlashString, UPNP_ERROR_MAP(XX))
#undef XX

#define XX(tag, text) DEFINE_FSTR_LOCAL(longstr##tag, text)
UPNP_ERROR_MAP(XX)
#undef XX

#define XX(tag, text) &longstr##tag,
DEFINE_FSTR_VECTOR(longStrings, FlashString, UPNP_ERROR_MAP(XX))
#undef XX

} // namespace

String toString(UPnP::Error error)
{
	auto code = int(error);
	return strings[code < 0 ? -code : 0];
}

String toLongString(UPnP::Error error)
{
	auto code = int(error);
	return longStrings[code < 0 ? -code : 0];
}
