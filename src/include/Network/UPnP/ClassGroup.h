/**
 * ClassGroup.h
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

#include "ObjectClass.h"
#include <WVector.h>

namespace UPnP
{
struct ClassGroup {
	class List : public Vector<ClassGroup>
	{
	public:
		int add(const FlashString& domain, const ObjectClass::List& classes);
		const ObjectClass* find(const Urn& objectType) const;
	};

	const FlashString& domain;
	const ObjectClass::List& classes;

	const ObjectClass* find(Urn::Kind kind, const String& type, uint8_t version) const;
};

} // namespace UPnP
