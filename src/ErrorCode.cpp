/**
 * ErrorCode.cpp
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

#include "include/Network/UPnP/ErrorCode.h"
#include <FlashString/Map.hpp>

namespace
{
using namespace UPnP;

#define XX(code, tag, text, description) DEFINE_FSTR_LOCAL(str##tag, #tag)
UPNP_ERRORCODE_MAP(XX)
#undef XX

#define XX(code, tag, text, description) {ErrorCode::tag, &str##tag},
DEFINE_FSTR_MAP(strings, ErrorCode, FlashString, UPNP_ERRORCODE_MAP(XX))
#undef XX

#define XX(code, tag, text, description) DEFINE_FSTR_LOCAL(longstr##tag, text)
UPNP_ERRORCODE_MAP(XX)
#undef XX

#define XX(code, tag, text, description) {ErrorCode::tag, &longstr##tag},
DEFINE_FSTR_MAP(longStrings, ErrorCode, FlashString, UPNP_ERRORCODE_MAP(XX))
#undef XX

} // namespace

String toString(ErrorCode error)
{
	auto m = strings[error];
	return m ? String(m) : F("ErrorCode{") + int(error) + '}';
}

String toLongString(ErrorCode error)
{
	auto m = longStrings[error];
	return m ? String(m) : F("ErrorCode{") + int(error) + '}';
}
