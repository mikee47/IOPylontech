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
void Request::setResponse(const void* buffer, size_t length)
{
    response.setString(static_cast<const char*>(buffer), length);
}

void Request::getJson(JsonObject json) const
{
    json["response"] = response;
}

} // namespace IO::RS485::Pylontech
