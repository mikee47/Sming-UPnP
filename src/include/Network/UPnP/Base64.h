/****
 * Base64.h
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

#pragma once

#include <Data/WebHelpers/base64.h>
#include <cstring>

namespace UPnP
{
class Base64
{
public:
	Base64()
	{
	}

	Base64(const String& data) : data(data)
	{
	}

	operator String() const
	{
		return data;
	}

	Base64& operator=(const char* value)
	{
		if(value == nullptr) {
			data = nullptr;
		} else {
			data = base64_decode(value, strlen(value));
		}
		return *this;
	}

	explicit operator bool() const
	{
		return bool(data);
	}

	String encode() const
	{
		return base64_encode(data);
	}

private:
	String data;
};

} // namespace UPnP
