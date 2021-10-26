/****
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
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include <FlashString/String.hpp>

namespace UPnP
{
DECLARE_FSTR(fs_xmlns)

namespace schemas_upnp_org
{
DECLARE_FSTR(device_1_0)
DECLARE_FSTR(service_1_0)
DECLARE_FSTR(control_1_0)

namespace device
{
DECLARE_FSTR(Basic)
}

} // namespace schemas_upnp_org

} // namespace UPnP
