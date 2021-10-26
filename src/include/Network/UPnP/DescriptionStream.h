/****
 * DescriptionStream.h
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

#pragma once

#include "Object.h"
#include <Data/Stream/DataSourceStream.h>

namespace UPnP
{
class DescriptionStream : public IDataSourceStream
{
public:
	/**
	 * @brief Construct a description stream
	 * @param object The Object to enumerate
	 */
	DescriptionStream(Object& object, const String& descriptionUrl) : object_(object)
	{
		setName(descriptionUrl);
		segments[0].item = &object;
		getContent();
	}

	~DescriptionStream()
	{
		freeMem();
	}

	/**
	 * @brief Reset back to start
	 * @note Handy if you want to re-use this stream to send it somewhere else
	 */
	void reset();

	bool isValid() const override
	{
		return true;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override;

	bool seek(int len) override;

	bool isFinished() override
	{
		return !content && (state == State::done);
	}

	String getName() const override
	{
		return name;
	}

	MimeType getMimeType() const override
	{
		return MimeType::XML;
	}

protected:
	void freeMem();
	void getContent();
	void setName(const String& descriptionUrl);

private:
	const Object& object_;
	String name;
	// Nesting levels
	struct Segment {
		Item* item{nullptr};
		String footer;
		ItemEnumerator* list{nullptr}; // active list
		String listName;
		uint8_t listIndex{0};
	};
	Segment segments[4];
	String content; // Buffer for current segment being output
	uint16_t readPos{0};
	enum class State {
		header,
		item,
		nextList,
		listItem,
		nextListItem,
		done,
	} state = State::header;
	uint8_t segIndex{0}; // nesting level
};

} // namespace UPnP
