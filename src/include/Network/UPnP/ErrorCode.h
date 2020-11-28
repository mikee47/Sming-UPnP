/**
 * ErrorCode.h
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

/**
 * @brief UPnP defined action error codes
 */
#define UPNP_ERRORCODE_MAP(XX)                                                                                         \
	XX(0, None, "No Error", "")                                                                                        \
	XX(401, InvalidAction, "Invalid Action", "No action by that name at this service")                                 \
	XX(402, InvalidArgs, "Invalid Args",                                                                               \
	   "Could be any of the following:\n"                                                                              \
	   "  - Not enough in args\n"                                                                                      \
	   "  - Args in the wrong order\n"                                                                                 \
	   "  - One or more in args are of the wrong data type\n"                                                          \
	   "Additionally, the UPnP Certification Test Tool shall return the following warning"                             \
	   " message if there are too many in args:\n"                                                                     \
	   "  'Sending too many in args is not recommended and may cause unexpected results.'")                            \
	XX(501, ActionFailed, "Action Failed", "Current state of service prevents invoking the action")                    \
	XX(600, ArgumentValueInvalid, "Argument Value Invalid", "The argument value is invalid")                           \
	XX(601, ArgumentValueOutOfRange, "Argument Value Out of Range",                                                    \
	   "An argument value is less than the minimum or more than the maximum value"                                     \
	   " of the allowed value range, or is not in the allowed value list.")                                            \
	XX(602, OptionalActionNotImplemented, "Optional Action Not Implemented",                                           \
	   "The requested action is optional and is not implemented by the device")                                        \
	XX(603, OutOfMemory, "Out of Memory",                                                                              \
	   "The device does not have sufficient memory available to complete the action."                                  \
	   " This is allowed to be a temporary condition; the control point is allowed to choose to retry"                 \
	   " the unmodified request again later and it is expected to succeed if memory is available.")                    \
	XX(604, HumanInterventionRequired, "Human Intervention Required",                                                  \
	   "The device has encountered an error condition which it cannot resolve"                                         \
	   " itself and required human intervention such as a reset or power cycle."                                       \
	   " See the device display or documentation for further guidance.")                                               \
	XX(605, StringArgumentTooLong, "String Argument Too Long",                                                         \
	   "A string argument is too long for the device to handle properly")

namespace UPnP
{
enum class ErrorCode {
#define XX(code, tag, text, description) tag = code,
	UPNP_ERRORCODE_MAP(XX)
#undef XX
};

} // namespace UPnP

String toString(UPnP::ErrorCode error);
String toLongString(UPnP::ErrorCode error);
