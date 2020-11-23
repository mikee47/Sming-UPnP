/**
 * DescriptionStream.cpp
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

#include "include/Network/UPnP/DescriptionStream.h"
#include "include/Network/UPnP/ItemEnumerator.h"
#include "include/Network/UPnP/RootDevice.h"

#define XML_PRETTY true

namespace UPnP
{
const SpecVersion specVersion{1, 0};

void DescriptionStream::reset()
{
	freeMem();
	state = State::header;
	readPos = 0;
	segIndex = 0;
	getContent();
}

void DescriptionStream::freeMem()
{
	for(unsigned i = 0; i <= segIndex; ++i) {
		auto& seg = segments[i];
		if(seg.list == nullptr) {
			continue;
		}
		delete seg.list;
		seg.list = nullptr;
		seg.listName = nullptr;
	}
}

void DescriptionStream::setName(const String& descriptionUrl)
{
	int i = descriptionUrl.lastIndexOf('/');
	if(i >= 0) {
		name = descriptionUrl.substring(i + 1);
	} else {
		name = descriptionUrl;
	}
}

void DescriptionStream::getContent()
{
	auto seg = &segments[segIndex];

	XML::Document doc;

	// Find closing tag and move it into the footer
	auto splitTag = [&](XML::Node* node) {
		String tag;
		tag.reserve(3 + node->name_size());
		tag += "</";
		tag.concat(node->name(), node->name_size());
		tag += '>';
		int i = content.indexOf(tag);
		assert(i >= 0);
		seg->footer = content.substring(i) + "\r\n" + seg->footer;
		content.remove(i);
#if XML_PRETTY
		content += "\r\n";
#endif
	};

	content.setLength(0);
	for(;;) {
		switch(state) {
		/*
		 * Main body of content, up to start of closing tag, e.g. "<root>..."
		 * Closing tag is inserted at start of footer, e.g. "</root>"
		 */
		case State::header: {
			XML::insertDeclaration(doc);
			auto node = seg->item->getDescription(doc, DescType::header);

			auto ver = XML::appendNode(node, _F("specVersion"));
			XML::appendNode(ver, _F("major"), specVersion.major);
			XML::appendNode(ver, _F("minor"), specVersion.minor);

			XML::serialize(doc, content, XML_PRETTY);
			splitTag(node);
			state = State::item;
			break;
		}

		/*
		 * Get item content, up to start of closing tag, e.g. "<device>..."
		 * Closing tag is inserted at start of footer, e.g. "</device>"
		 */
		case State::item: {
			state = State::nextList;

			auto node = seg->item->getDescription(doc, segIndex == 0 ? DescType::content : DescType::embedded);
			if(node == nullptr) {
				continue;
			}

			XML::serialize(doc, content);
			splitTag(node);
			seg->listIndex = 0;
			break;
		}

		/*
		 * Open the next list
		 */
		case State::nextList: {
			assert(seg->list == nullptr);
			seg->list = seg->item->getList(seg->listIndex, seg->listName);
			if(seg->list == nullptr) {
				// No more lists, emit item footer
				if(seg->footer) {
					content = seg->footer;
					seg->footer = nullptr;
				}
				if(segIndex == 0) {
					// all done
					state = State::done;
				} else {
					// next item from previous level
					state = State::nextListItem;
				}
				if(content.length() == 0) {
					continue;
				}
				break;
			}

			if(seg->list->current() != nullptr) {
				// Emit list header
				content = '<';
				content += seg->listName;
				content += '>';
#ifdef XML_PRETTY
				content += "\r\n";
#endif
				state = State::listItem;
				break;
			}

			// Empty list (todo: omit list entirely when it's all working)
			content = '<';
			content += seg->listName;
			content += "/>";
#ifdef XML_PRETTY
			content += "\r\n";
#endif
			delete seg->list;
			seg->list = nullptr;
			++seg->listIndex;
			break;
		}

		// drop down into the current list item
		case State::listItem:
			++segIndex;
			assert(segIndex < ARRAY_SIZE(segments));
			segments[segIndex].item = seg->list->current();
			++seg;
			state = State::item;
			continue;

		// back up a level to fetch next list item
		case State::nextListItem: {
			assert(segIndex > 0);
			--segIndex;
			--seg;

			if(seg->list == nullptr) {
				state = State::nextList;
				continue;
			}

			if(seg->list->next() != nullptr) {
				state = State::listItem;
				continue;
			}

			// end of list
			content = "</";
			content += seg->listName;
			content += '>';
#ifdef XML_PRETTY
			content += "\r\n";
#endif
			delete seg->list;
			seg->list = nullptr;
			++seg->listIndex;
			state = State::nextList;
			break;
		}

		case State::done:
			content = nullptr;
		}

		return;
	}
}

uint16_t DescriptionStream::readMemoryBlock(char* data, int bufSize)
{
	if(bufSize <= 0) {
		return 0;
	}

	if(content) {
		auto len = std::min(size_t(bufSize), content.length() - readPos);
		memcpy(data, &content[readPos], len);
		return len;
	}

	return 0;
}

bool DescriptionStream::seek(int len)
{
	if(len <= 0 || content.length() == 0) {
		return false;
	}

	unsigned newPos = readPos + len;
	if(newPos > content.length()) {
		debug_e("[UPnP] seek(%d) out of range, max %u", len, content.length());
		return false;
	}

	if(newPos < content.length()) {
		readPos = newPos;
		return true;
	}

	getContent();
	readPos = 0;
	return true;
}

} // namespace UPnP
