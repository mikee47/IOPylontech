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

#include "include/Pylontech/Tables.h"

#ifdef ARCH_HOST
#include <FlashString/Stream.hpp>
#endif

namespace
{
// Name, Rename-As, Example Value
#define PWR_FIELD_MAP(XX)                                                                                              \
	XX("Power", "Battery", "1")                                                                                        \
	XX("Volt", "Voltage", "49939")                                                                                     \
	XX("Curr", "Current", "-1187")                                                                                     \
	XX("Tempr", "Temp", "22000")                                                                                       \
	XX("Tlow", "Tlow", "19700")                                                                                        \
	XX("Thigh", "Thigh", "20200")                                                                                      \
	XX("Vlow", "Vlow", "3328")                                                                                         \
	XX("Vhigh", "Vhigh", "3330")                                                                                       \
	XX("Base.St", "Base State", "Dischg")                                                                              \
	XX("Volt.St", "Volt State", "Normal")                                                                              \
	XX("Curr.St", "Curr State", "Normal")                                                                              \
	XX("Temp.St", "Temp State", "Normal")                                                                              \
	XX("Coulomb", "SOC", "91%")                                                                                        \
	XX("Time", "Time", "2023-09-21 17:59:01")                                                                          \
	XX("B.V.St", "BV State", "Normal")                                                                                 \
	XX("B.T.St", "BT State", "Normal")                                                                                 \
	XX("MosTempr", "MOS Temp", "20700")                                                                                \
	XX("M.T.St", "MT State", "Normal")

// Name, Rename-As, Example Value
#define BAT_FIELD_MAP(XX)                                                                                              \
	XX("Battery", "Cell", "0")                                                                                         \
	XX("Volt", "Voltage", "3329")                                                                                      \
	XX("Curr", "Current", "-1186")                                                                                     \
	XX("Tempr", "Temp", "20100")                                                                                       \
	XX("Base State", "Base State", "Dischg")                                                                           \
	XX("Volt. State", "Volt State", "Normal")                                                                          \
	XX("Curr. State", "Curr State", "Normal")                                                                          \
	XX("Temp. State", "Temp State", "Normal")                                                                          \
	XX("SOC", "SOC", "92%")                                                                                            \
	XX("Coulomb", "Coulomb", "66561 mAH")                                                                              \
	XX("BAL", "Balance", "N")

#define XX(from, to, ...) from "=" to "\0"
DEFINE_FSTR(pwr_field_map, PWR_FIELD_MAP(XX))
DEFINE_FSTR(bat_field_map, BAT_FIELD_MAP(XX))
#undef XX

size_t splitLine(char* line, size_t len, bool doubleSep = false)
{
	bool sep{false};
	const char* src = line;
	const char* end = src + len;
	auto dst = line;
	while(src < end) {
		if(sep && *src == ' ') {
			++src;
			continue;
		}
		auto skip = [&](size_t len) {
			src += len;
			*dst++ = '\0';
			sep = true;
		};
		if(src[0] == '%' && src[1] == ' ') {
			skip(2);
			continue;
		}
		if(memcmp(src, " mAH ", 5) == 0) {
			skip(5);
			continue;
		}
		if(doubleSep && src[0] == ' ' && src[1] == ' ') {
			skip(2);
			continue;
		}
		if(!doubleSep && src[0] == ' ') {
			skip(1);
			continue;
		}
		*dst++ = *src++;
		sep = false;
	}
	return dst - line;
}

} // namespace

namespace IO::RS485::Pylontech
{
size_t Table::printTo(Print& p) const
{
	size_t n = p.println(headings.join());
	for(auto row : rows) {
		n += p.println(row.join());
	}
	return n;
}

void processResponseTable(ResponseReader reader, Table& table, bool doubleSep)
{
	table.clear();
	enum class State {
		start,
		heading,
		rows,
	};
	State state{};
	char line[256];
	size_t buflen{0};
	for(;;) {
		buflen += reader(&line[buflen], sizeof(line) - 1 - buflen);
		line[buflen] = '\0';
		auto end = strpbrk(line, "\r\n");
		if(!end) {
			return;
		}
		*end = '\0';
		switch(state) {
		case State::start:
			if(strcmp(line, "@") == 0) {
				state = State::heading;
			}
			break;

		case State::heading: {
			auto len = splitLine(line, end - line, doubleSep);
			table.headings = CStringArray(line, len);
			state = State::rows;
			break;
		}

		case State::rows: {
			if(!isdigit(line[0])) {
				return;
			}
			auto len = splitLine(line, end - line, doubleSep);
			CStringArray row(line, len);
			if(memcmp(row[1], "-", 2) == 0) {
				break;
			}
			table.rows.add(row);
			break;
		}
		}
		size_t len = 1 + end - line;
		while(len < buflen && (line[len] == '\r' || line[len] == '\n')) {
			++len;
		}
		memmove(line, &line[len], buflen - len);
		buflen -= len;
	}
}

bool fixupHeadings(CStringArray& headings, const CStringArray& map)
{
	CStringArray res;
	auto itMap = map.begin();
	for(auto heading : headings) {
		auto from = *itMap;
		auto sep = strchr(from, '=');
		assert(sep);
		auto to = sep + 1;
		if(memcmp(heading, from, sep - from) != 0) {
			m_printf("MISMATCH %s / %s\r\n", heading, from);
			return false;
		}
		res += to;
		++itMap;
	}

	headings = res;
	return true;
}

bool fixupTablePwr(Table& table)
{
	if(!fixupHeadings(table.headings, pwr_field_map)) {
		return false;
	}

	int timeCol = table.headings.indexOf("Time");
	if(timeCol < 0) {
		return false;
	}

	bool isok{true};
	for(auto& row : table.rows) {
		auto timeSep = row[timeCol + 1];
		if(timeSep) {
			*const_cast<char*>(timeSep - 1) = ' ';
		} else {
			isok = false;
		}
	}

	return isok;
}

bool fixupTableBat(Table& table)
{
	return fixupHeadings(table.headings, bat_field_map);
}

#ifdef ARCH_HOST
IMPORT_FSTR(rsp_pwr, COMPONENT_PATH "/data/rsp-pwr.txt")
IMPORT_FSTR(rsp_bat_1, COMPONENT_PATH "/data/rsp-bat-1.txt")

bool loadTablePwr(Table& table)
{
	FSTR::Stream stream(rsp_pwr);
	auto reader = [&stream](char* buffer, size_t len) { return stream.readBytes(buffer, len); };
	processResponseTable(reader, table, false);
	return fixupTablePwr(table);
}

bool loadTableBat(Table& table)
{
	FSTR::Stream stream(rsp_bat_1);
	auto reader = [&stream](char* buffer, size_t len) { return stream.readBytes(buffer, len); };
	processResponseTable(reader, table, true);
	return fixupTableBat(table);
}
#endif

} // namespace IO::RS485::Pylontech
