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
#include "Tables.h"

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

	void getJson(JsonObject json) const override;

	/**
	 * @brief Identifies which battery to talk to
	 * @param node DevNode_ALL or 0 implies a 'PWR' command, otherwise 'BAT'
	 *
	 * PWR contains summary information for all batteries
	 * BAT obtains details for a specific battery
	 */
	bool setNode(DevNode node) override
	{
		this->node = node.id;
		return true;
	}

	/**
	 * @brief Provide an arbitrary command line
	 */
	void setCmdLine(const String& s)
	{
		cmdline = s;
		setCommand(Command::undefined);
	}

	String getCmdLine() const;

	/**
	 * @brief Parse serial response into a table
	 */
	ErrorCode processResponse(ResponseReader reader);

	/**
	 * @brief Get the parsed table response
	 */
	const Table& getTable() const
	{
		return table;
	}

private:
	String cmdline;
	DevNode::ID node{};
	Table table;
};

} // namespace IO::RS485::Pylontech
