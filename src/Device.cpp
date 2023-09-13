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
	auto& ctrl = static_cast<IO::RS485::Controller&>(controller);
	ctrl.getSerial().resizeBuffers(256, 256);
	return IO::RS485::Device::init(config);
}

ErrorCode Device::init(JsonObjectConst config)
{
	Config cfg{};
	parseJson(config, cfg);
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
			return;
		}
		break;
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
	String cmd = F("~200246470000FDA7\r");

	// Prepare UART for comms
	IO::Serial::Config cfg{
		.baudrate = baudrate(),
		.format = UART_8N1,
	};
	auto& serial = getController().getSerial();
	serial.setConfig(cfg);
	serial.clear();

	// OK, issue the request
	getController().send(cmd.c_str(), cmd.length());
	request->setResponse("empty", 5);
	return Error::pending;
}

ErrorCode Device::readResponse(Request* request)
{
	// Read packet
	auto& serial = getController().getSerial();
	uint8_t buffer[256];
	auto receivedSize = serial.read(buffer, sizeof(buffer));
	request->setResponse(buffer, receivedSize);

	return Error::success;
}

} // namespace IO::RS485::Pylontech
