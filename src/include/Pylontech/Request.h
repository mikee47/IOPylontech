/**
 * Modbus/Pylontech/Request.h
 *
 * Copyright 2023 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the IOPylontech Library
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

#include <IO/RS485/Request.h>
#include "Device.h"

namespace IO::RS485::Pylontech
{
class Request : public RS485::Request
{
public:
	Request(Device& device) : RS485::Request(device)
	{
	}

	Device& getDevice() const
	{
		return static_cast<Device&>(device);
	}

	void setResponse(const void* buffer, size_t length);

	void getJson(JsonObject json) const override;

	bool setNode(DevNode node) override
	{
		return true;
	}

private:
	String response;
};

} // namespace IO::RS485::Pylontech
