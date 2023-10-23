/**
 * Modbus/Pylontech/Device.cpp
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

#include "include/Pylontech/Device.h"
#include "include/Pylontech/Request.h"

namespace IO::RS485::Pylontech
{
const Device::Factory Device::factory;

ErrorCode Device::init(const Config& config)
{
	auto err = RS485::Device::init(config);
	if(err) {
		return err;
	}

	// Prepare UART for comms - port isn't shared so only need to do this once
	IO::Serial::Config cfg{
		.baudrate = baudrate(),
		.format = UART_8N1,
	};
	auto& serial = getController().getSerial();
	serial.resizeBuffers(2800, 0);
	serial.setConfig(cfg);

	return Error::success;
}

ErrorCode Device::init(JsonObjectConst json)
{
	Config cfg{};
	parseJson(json, cfg);
	return init(cfg);
}

IO::Request* Device::createRequest()
{
	return new Request(*this);
}

void Device::handleEvent(IO::Request* request, Event event)
{
	auto req = static_cast<Request*>(request);

	switch(event) {
	case Event::Execute: {
		IO::RS485::Device::handleEvent(request, event);
		ErrorCode err = execute(req);
		if(err != Error::pending) {
			request->complete(err);
		}
		return;
	}

	case Event::ReceiveComplete: {
		auto err = readResponse(req);
		if(err != Error::pending) {
			request->complete(err);
		}
		break;
	}

	case Event::TransmitComplete:
	case Event::Timeout:
		break;

	case Event::RequestComplete:
		break;
	}

	IO::RS485::Device::handleEvent(request, event);
}

ErrorCode Device::execute(Request* request)
{
	// Issue the request
	auto& serial = getController().getSerial();
	String cmd = request->getCmdLine();
	serial.write(cmd.c_str(), cmd.length());
	serial.write("\r", 1);
	return Error::pending;
}

ErrorCode Device::readResponse(Request* request)
{
	auto& serial = getController().getSerial();
	auto reader = [&serial](char* buffer, size_t len) { return serial.read(buffer, len); };
	return request->processResponse(reader);
}

} // namespace IO::RS485::Pylontech
