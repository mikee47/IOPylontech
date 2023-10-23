/**
 * Modbus/Pylontech/Request.cpp
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

#include "include/Pylontech/Request.h"
#include <IO/Strings.h>

namespace IO::RS485::Pylontech
{
String Request::getCmdLine() const
{
	if(getCommand() != Command::query) {
		return cmdline;
	}

	if(node == DevNode_ALL.id || node == 0) {
		return "pwr";
	}

	return String("bat ") + node;
}

ErrorCode Request::processResponse(ResponseReader reader)
{
	if(getCommand() != Command::query) {
		// Use cmdline
		return Error::success;
	}

	if(node == DevNode_ALL.id || node == 0) {
		processResponseTable(reader, table, false);
		return fixupTablePwr(table) ? Error::success : Error::bad_size;
	}

	processResponseTable(reader, table, true);
	return fixupTableBat(table) ? Error::success : Error::bad_size;
}

void Request::getJson(JsonObject json) const
{
	RS485::Request::getJson(json);
	if(!table.headings) {
		return;
	}
	auto response = json.createNestedObject("response");

	auto getCols = [](const CStringArray& row) {
		auto& rs = reinterpret_cast<const String&>(row);
		int i = rs.indexOf('\0');
		String s = rs.substring(i + 1, rs.length());
		s.replace('\0', ',');
		return s;
	};
	response["headings"] = getCols(table.headings);
	for(auto row : table.rows) {
		response[String(row[0])] = getCols(row);
	}
}

} // namespace IO::RS485::Pylontech
