/**
 * Urn.h - Construction of device/service URNs
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

#pragma once

#include <WString.h>

#define UPNP_URN_KIND_MAP(XX)                                                                                          \
	XX(none)                                                                                                           \
	XX(device)                                                                                                         \
	XX(service)

namespace UPnP
{
/**
 * @brief Structure for UPnP URNs
 */
class Urn
{
public:
	enum class Kind {
#define XX(tag) tag,
		UPNP_URN_KIND_MAP(XX)
#undef XX
	};

	Urn()
	{
	}

	Urn(Urn&& urn) : kind(urn.kind), domain(std::move(urn.domain)), type(std::move(urn.type)), version(urn.version)
	{
	}

	Urn(Kind kind, const String& domain, const String& type, uint8_t version)
		: kind(kind), domain(domain), type(type), version(version ?: 1)
	{
	}

	Urn& operator=(const Urn& urn)
	{
		kind = urn.kind;
		domain = urn.domain;
		type = urn.type;
		version = urn.version;
		return *this;
	}

	/**
	 * @brief Get URN string
	 *
	 * For example: "urn:upnp-org:service:Basic:1"
	 */
	String toString() const;

	operator String() const
	{
		return toString();
	}

	/**
	 * @brief Determine if URN is valid
	 */
	operator bool() const
	{
		return kind != Kind::none;
	}

	Kind kind{};
	String domain;		///< e.g. PnP::schemas_upnp_org
	String type;		///< e.g. "Basic"
	uint8_t version{1}; ///< e.g. 1
};

/**
 * @brief A UPnP Device URN
 */
class DeviceUrn : public Urn
{
public:
	DeviceUrn() : Urn()
	{
	}

	DeviceUrn(const String& domain, const String& type, const String& version)
		: Urn(Urn::Kind::device, domain, type, version.toInt())
	{
	}

	DeviceUrn(const String& domain, const String& type, uint8_t version) : Urn(Urn::Kind::device, domain, type, version)
	{
	}
};

/**
 * @brief A UPnP Service URN
 */
struct ServiceUrn : public Urn {
public:
	ServiceUrn() : Urn()
	{
	}

	ServiceUrn(const String& domain, const String& type, const String& version)
		: Urn(Urn::Kind::device, domain, type, version.toInt())
	{
	}

	ServiceUrn(const String& domain, const String& type, uint8_t version)
		: Urn(Urn::Kind::service, domain, type, version)
	{
	}
};

} // namespace UPnP

String toString(UPnP::Urn::Kind kind);

inline String toString(const UPnP::Urn& urn)
{
	return urn.toString();
}
