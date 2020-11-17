/**
 * Soap.h
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
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

#include <RapidXML.h>

namespace SOAP
{
/**
 * @brief Helper method to check value matches, log error if not
 * @retval bool true if OK, false if check failed
 * @note
 */
bool checkValue(const char* str1, size_t len1, const char* str2, size_t len2);

inline bool checkValue(const String& str, const char* value, size_t len)
{
	return checkValue(str.c_str(), str.length(), value, len);
}

bool checkNodeName(XML::Node* node, const String& str);

bool checkAttrValue(XML::Attribute* attr, const String& str);

XML::Node* getNode(XML::Node* parent, const String& name);

String getNodeValue(XML::Node* parent, const String& name);

XML::Node* addNodeValue(XML::Node* parent, const String& name, const String& value);

class Envelope
{
public:
	/**
	 * @brief Load a SOAP document
	 * @param content MUST remain valid for the lifetime of this Envelope,
	 * or until initialise() is called.
	 */
	bool load(String& content);

	bool load(const FlashString& content);

	String toString(bool pretty) const
	{
		return XML::serialize(doc, pretty);
	}

	bool initialise();

	XML::Node* body()
	{
		return body_;
	}

public:
	XML::Document doc;

private:
	XML::Node* findEnvelope();
	XML::Node* findBody();

	XML::Node* body_{nullptr};
};

} // namespace SOAP
