/**
 * Constants.cpp
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

#include "include/Network/UPnP/Constants.h"

namespace UPnP
{
DEFINE_FSTR(upnp_org, "upnp-org");
DEFINE_FSTR(schemas_upnp_org, "schemas-upnp-org");

namespace DeviceType
{
#define XX(tag, str) DEFINE_FSTR(tag, str)
UPNP_DEVICETYPE_MAP(XX)
#undef XX
} // namespace DeviceType

namespace ServiceType
{
namespace upnp_org
{
#define XX(tag, str) DEFINE_FSTR(tag, str)
UPNP_SERVICETYPE_MAP(XX)
#undef XX
} // namespace upnp_org
} // namespace ServiceType

} // namespace UPnP
