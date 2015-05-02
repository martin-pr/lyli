/*
 * This file is part of Usbpp, a C++ wrapper around libusb
 * Copyright (C) 2015  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
 *
 * Usbpp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3 of the License
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "msdevice.h"

#include "buffer.h"
#include "mscbw.h"
#include "mscsw.h"
#include "msscsiinquiry.h"
#include "msscsiinquiryresponse.h"

#include <libusb.h>
#include <memory>
#include <string>

namespace {

using namespace Usbpp;
using namespace Usbpp::MassStorage;

SCSI::InquiryResponse sendInquiry(const Device *device, unsigned char endpoint, uint8_t LUN, bool explicitpage, uint8_t page = 0) {
	unsigned char inEndpoint(endpoint | LIBUSB_ENDPOINT_IN);
	ByteBuffer tmpBuf(36);
	ByteBuffer statusBuf(13);

	// first send the inquiry with the minimal allocation length
	if (explicitpage) {
		SCSI::Inquiry inquiry1(LUN, 36, page);
		device->bulkTransferOut(endpoint, inquiry1.getBuffer(), 2000);
	}
	else {
		SCSI::Inquiry inquiry1(LUN, 36);
		device->bulkTransferOut(endpoint, inquiry1.getBuffer(), 2000);
	}
	int transferred = device->bulkTransferIn(inEndpoint, tmpBuf, 2000);
	if (transferred <= 36) {
		// TODO: throw
	}

	// read status
	device->bulkTransferIn(inEndpoint, statusBuf, 2000);

	SCSI::InquiryResponse response(ByteBuffer(tmpBuf.data(), transferred));

	if (response.getAdditionalLength() <= 31) {
		return response;
	}

	// send inquiry with the correct allocation length
	// the reason why 5 is added rather than 4 (as mentioned in specification)
	// is that there are 4 bytes in the header + 1 byte for the additional length itself
	uint16_t inquiryLength(5 + response.getAdditionalLength());
	tmpBuf.resize(inquiryLength);
	if (explicitpage) {
		SCSI::Inquiry inquiry2(LUN, inquiryLength, page);
		device->bulkTransferOut(endpoint, inquiry2.getBuffer(), 2000);
	}
	else {
		SCSI::Inquiry inquiry2(LUN, inquiryLength);
		device->bulkTransferOut(endpoint, inquiry2.getBuffer(), 2000);
	}
	transferred = device->bulkTransferIn(inEndpoint, tmpBuf, 2000);

	// read status
	device->bulkTransferIn(inEndpoint, statusBuf, 2000);

	if (transferred <= 36) {
		// return the previous result, to have at least something
		return response;
	}

	return SCSI::InquiryResponse(ByteBuffer(tmpBuf.data(), transferred));;
}

}

namespace Usbpp {
namespace MassStorage {

MSDevice::MSDevice() {

}

MSDevice::MSDevice(const Device& device): Device(device) {

}

MSDevice::MSDevice(const MSDevice& device) : Device(device) {

}

MSDevice::MSDevice(MSDevice&& device) noexcept: Device(std::move(device)) {

}

CommandStatusWrapper MSDevice::sendCommand(unsigned char endpoint, const CommandBlockWrapper& command, ByteBuffer *data)
{
	// an "in" endpoint derived from the endpoint
	unsigned char inEndpoint(endpoint | LIBUSB_ENDPOINT_IN);
	
	// send command
	bulkTransferOut(endpoint, command.getBuffer(), 2000);
	
	// if the command is an outgoing command that expects a response, read it
	if ((endpoint & LIBUSB_ENDPOINT_IN) == 0 && command.getTransferLength() != 0) {
		ByteBuffer tmpBuf(command.getTransferLength());
		int transferred = bulkTransferIn(inEndpoint, tmpBuf, 2000);
		if (data != nullptr) {
			tmpBuf.resize(transferred);
			*data = std::move(tmpBuf);
		}
	}
	
	// read status
	ByteBuffer tmpBuf(13);
	int transferred = bulkTransferIn(inEndpoint, tmpBuf, 2000);
	if (transferred == 13) {
		return CommandStatusWrapper(std::move(tmpBuf));
	}
	else {
		// TODO: throw
		return CommandStatusWrapper();
	}
}

SCSI::InquiryResponse MSDevice::sendInquiry(unsigned char endpoint, uint8_t LUN) const {
	return ::sendInquiry(this, endpoint, LUN, false);
}

SCSI::InquiryResponse MSDevice::sendInquiry(unsigned char endpoint, uint8_t LUN, uint8_t page) const
{
	return ::sendInquiry(this, endpoint, LUN, true, page);
}

}
}
