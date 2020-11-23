/**
 * DescriptionParser.h
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

#include <Data/Stream/ReadWriteStream.h>
#include "include/Network/UPnP/RootDeviceControl.h"
#include "XmlBuffer.h"

namespace UPnP
{
class ControlPoint;

class DescriptionParser : public ReadWriteStream
{
public:
	DescriptionParser(ControlPoint& controlPoint, const String& location)
		: controlPoint(controlPoint), location(location)
	{
	}

	~DescriptionParser()
	{
		delete rootDevice;
	}

	using ReadWriteStream::write;

	size_t write(const uint8_t* data, size_t size) override;

	uint16_t readMemoryBlock(char* buffer, int bufSize) override
	{
		return 0;
	}

	bool isFinished() override
	{
		return true;
	}

	int available() override
	{
		return 0;
	}

	/**
	 * Parser creates root device
	 */
	RootDeviceControl* rootDevice{nullptr};

private:
	// Identifies which section of description is being parsed
	enum class State {
		searching,
		found,
		done,
		error,
	};

	ControlPoint& controlPoint;
	String location;
	State state{};
	Tag foundTag;
	XmlBuffer buffer;
	size_t totalSize{0};
	DeviceControl* device{nullptr}; // Current device being processed
};

} // namespace UPnP
