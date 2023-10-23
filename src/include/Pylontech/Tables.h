/**
 * Modbus/Pylontech/Tables.cpp
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

#include <Data/CStringArray.h>
#include <WVector.h>
#include <Print.h>
#include <Delegate.h>

namespace IO::RS485::Pylontech
{
struct Table {
	CStringArray headings;
	Vector<CStringArray> rows;

	void clear()
	{
		headings.clear();
		rows.clear();
	}

	size_t printTo(Print& out) const;
};

using ResponseReader = Delegate<size_t(char* buffer, size_t len)>;

/**
 * @brief Parse tabular response into a `Table` structure
 * @param reader Callback to get block of response data
 * @param table OUT place to store parsed table data
 * @param doubleSep When false, table columns are separated by one or more spaces.
 * When true, two or more spaces are required since some column values themselves contain a space.
 */
void processResponseTable(ResponseReader reader, Table& table, bool doubleSep);

/**
 * @brief Perform any adjustments to a parsed table received for a PWR command
 * @retval bool true on success, false if table format invalid
 *
 * - Normalise column names
 * - Fix the 'Time' field values which contains a space
 */
bool fixupTablePwr(Table& table);

/**
 * @brief Perform any adjustments to a parsed table received for a BAT command
 * @retval bool true on success, false if table format invalid
 *
 * - Normalise column names
 */
bool fixupTableBat(Table& table);

#ifdef ARCH_HOST
bool loadTablePwr(Table& table);
bool loadTableBat(Table& table);
#endif

} // namespace IO::RS485::Pylontech
