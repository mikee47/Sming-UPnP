/**
 * Error.h
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

#include <WString.h>

#define UPNP_ERROR_MAP(XX)                                                                                             \
	XX(Success, "Success")                                                                                             \
	XX(NoMemory, "Insufficient memory")                                                                                \
	XX(XmlParsing, "XML paring failed")                                                                                \
	XX(NoSoapBody, "SOAP Body missing")                                                                                \
	XX(NoActiveRequest, "No active request")                                                                           \
	XX(NoSoapContent, "SOAP content missing")                                                                          \
	XX(BadSoapFault, "Unknown SOAP fault kind")                                                                        \
	XX(BadSoapNamespace, "Bad SOAP namespace attribute")

namespace UPnP
{
enum class Error {
#define XX(tag, comment) tag,
	UPNP_ERROR_MAP(XX)
#undef XX
};

} // namespace UPnP

inline bool operator!(UPnP::Error error)
{
	return error == UPnP::Error::Success;
}

String toString(UPnP::Error error);
String toLongString(UPnP::Error error);
