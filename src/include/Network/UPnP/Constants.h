/**
 * Constants.h
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
 * You should have received a copy of the GNU General Public License along with FlashString.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include <FlashString/String.hpp>

namespace UPnP
{
DECLARE_FSTR(upnp_org);
DECLARE_FSTR(schemas_upnp_org);

#define UPNP_DEVICETYPE_MAP(XX) XX(Basic, "Basic")

namespace DeviceType
{
#define XX(tag, str) DECLARE_FSTR(tag)
UPNP_DEVICETYPE_MAP(XX)
#undef XX
} // namespace DeviceType

#define UPNP_SERVICETYPE_MAP(XX)                                                                                       \
	XX(ContentDirectory, "ContentDirectory")                                                                           \
	XX(ConnectionManager, "ConnectionManager")                                                                         \
	XX(X_MS_MediaReceiverRegistrar, "X_MS_MediaReceiverRegistrar")

namespace ServiceType
{
namespace upnp_org
{
#define XX(tag, str) DECLARE_FSTR(tag)
UPNP_SERVICETYPE_MAP(XX)
#undef XX
} // namespace upnp_org
} // namespace ServiceType

} // namespace UPnP
