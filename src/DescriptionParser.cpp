/**
 * DescriptionParser.cpp
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
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "DescriptionParser.h"
#include "include/Network/UPnP/ControlPoint.h"
#include "XmlBuffer.h"

namespace UPnP
{
size_t DescriptionParser::write(const uint8_t* data, size_t size)
{
	size_t result = size;
	totalSize += size;

	auto startLen = buffer.length();

	if(!buffer.concat(reinterpret_cast<const char*>(data), size)) {
		state = State::error;
		return result;
	}

	while(size != 0 && state < State::done) {
		auto match = buffer.nextTag();
		if(!match) {
			break;
		}
		if(match.pos > startLen) {
			auto len = match.pos - startLen;
			data += len;
			size -= len;
			startLen += len;
		}

		if(state == State::searching) {
			if(!match.close && (Tag::device == match.tag || Tag::service == match.tag)) {
				state = State::found;
				foundTag = match.tag;
				buffer.setStart(match.pos);
			}
			continue;
		}

		if(!match.close || match.tag != foundTag) {
			if(!(foundTag == Tag::device && (match.tag == Tag::iconList || match.tag == Tag::serviceList))) {
				continue;
			}
		}

		String tagString = tags[unsigned(foundTag)];

		if(!buffer.reserve(match.pos + 3 + deviceTag.length())) {
			state = State::error;
			break;
		}
		auto len = match.pos;
		buffer[len++] = '<';
		buffer[len++] = '/';
		strcpy(&buffer[len], tagString.c_str());
		len += deviceTag.length();
		buffer[len++] = '>';
		buffer[len++] = '\0';

		XML::Document doc;
		XML::deserialize(doc, buffer.begin());

#if DEBUG_VERBOSE_LEVEL >= DBG
		String s = XML::serialize(doc, true);
		m_puts("\r\n");
		m_puts("Section:\r\n");
		m_nputs(s.c_str(), s.length());
		m_puts("\r\n");
		m_puts("\r\n");
#endif

		auto objectNode = XML::getNode(doc, tagString);
		String objectType = XML::getValue(objectNode, tagString + F("Type"));
		auto cls = ControlPoint::findClass(Urn(objectType));
		if(cls == nullptr) {
			if(rootDevice == nullptr) {
				state = State::error;
				break;
			}
		} else if(foundTag == Tag::device) {
			if(rootDevice == nullptr) {
				auto dev = cls->createRootDevice();
				if(dev->configure(controlPoint, location, objectNode)) {
					debug_i("Configured root device '%s'", dev->caption().c_str());
					rootDevice = dev;
					device = dev;
				} else {
					delete dev;
					state = State::error;
					break;
				}
			} else {
				assert(device != nullptr);
				auto parent = device;
				device = cls->createDevice(*parent);
				if(device->configure(objectNode)) {
					parent->addDevice(device);
					debug_i("Configured device '%s'", device->caption().c_str());
				} else {
					delete device;
				}
			}
		} else if(device != nullptr) {
			auto service = cls->createService(*device);
			device->addService(service);
			service->configure(objectNode);
			debug_i("Configured service '%s' on device '%s'", service->caption().c_str(),
					device->friendlyName().c_str());
		}

		buffer.setStart(match.pos);
		state = State::searching;

		if(match.close && Tag::device == match.tag) {
			device = &device->parent();
		}
	}

	return result;
}

} // namespace UPnP
