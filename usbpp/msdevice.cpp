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
#include "cbw.h"
#include "csw.h"
#include "scsiinquiry.h"
#include "scsiinquiryresponse.h"

#include <memory>
#include <string>

namespace Usbpp {
namespace MassStorage {

MSDevice::MSDevice(const Device& device): Device(device)
{

}

MSDevice::MSDevice(const MSDevice& device) : Device(device)
{

}

MSDevice::MSDevice(MSDevice&& device) : Device(std::move(device))
{

}

CommandStatusWrapper MSDevice::sendCommand(unsigned char endpoint, const CommandBlockWrapper& command, ByteBuffer *data)
{
	int transferred;
	// an "in" endpoint derived from the endpoint
	unsigned char inEndpoint(endpoint | LIBUSB_ENDPOINT_IN);
	
	// send command
	bulkTransfer(endpoint, command.getData(), command.getDataLength(), &transferred, 2000);
	
	// if the command is an outgoing command that expects a response, read it
	if ((endpoint & LIBUSB_ENDPOINT_IN) == 0 && command.getTransferLength() != 0) {
		unsigned char tmpBuf[command.getTransferLength()];
		bulkTransfer(inEndpoint, tmpBuf, command.getTransferLength(), &transferred, 2000);
		if (data != nullptr) {
			*data = ByteBuffer(tmpBuf, transferred);
		}
	}
	
	// read status
	unsigned char tmpBuf[13];
	bulkTransfer(inEndpoint, tmpBuf, 13, &transferred, 2000);
	if (transferred == 13) {
		return CommandStatusWrapper(ByteBuffer(tmpBuf, transferred));
	}
	else {
		// TODO: throw
		return CommandStatusWrapper();
	}
}

SCSI::InquiryResponse MSDevice::sendInquiry(unsigned char endpoint, uint8_t LUN) const
{
	int transferred, statusTransferred;
	unsigned char inEndpoint(endpoint | LIBUSB_ENDPOINT_IN);
	std::unique_ptr<unsigned char[]> tmpBuf(new unsigned char[36]);
	unsigned char statusBuf[13];

	// first send the inquiry with the minimal allocation length
	SCSI::Inquiry inquiry1(LUN, 36);
	bulkTransfer(endpoint, inquiry1.getData(), inquiry1.getDataLength(), &transferred, 2000);
	bulkTransfer(inEndpoint, tmpBuf.get(), 36, &transferred, 2000);
	
	// read status
	bulkTransfer(inEndpoint, statusBuf, 13, &statusTransferred, 2000);
	
	if (transferred <= 36) {
		// TODO: throw
	}
	
	SCSI::InquiryResponse response(ByteBuffer(tmpBuf.get(), transferred));
	
	if (response.getAdditionalLength() <= 31) {
		return response;
	}
	
	// send inquiry with the correct allocation length
	// the reason why 5 is added rather than 4 (as mentioned in specification)
	// is that there are 4 bytes in the header + 1 byte for the additional length itself
	uint16_t inquiryLength(5 + response.getAdditionalLength());
	tmpBuf.reset(new unsigned char[inquiryLength]);
	SCSI::Inquiry inquiry2(LUN, inquiryLength);
	bulkTransfer(endpoint, inquiry2.getData(), inquiry2.getDataLength(), &transferred, 2000);
	bulkTransfer(inEndpoint, tmpBuf.get(), inquiryLength, &transferred, 2000);
	
	// read status
	bulkTransfer(inEndpoint, statusBuf, 13, &statusTransferred, 2000);
	
	if (transferred <= 36) {
		// return the previous result, to have at least something
		return response;
	}
	
	response = std::move(ByteBuffer(tmpBuf.get(), transferred));
	
	return response;
}

// pretty much a copy of the previous sendInquiry function
// TODO: try to avoid this using the std::bind
SCSI::InquiryResponse MSDevice::sendInquiry(unsigned char endpoint, uint8_t LUN, uint8_t page) const
{
	int transferred, statusTransferred;
	unsigned char inEndpoint(endpoint | LIBUSB_ENDPOINT_IN);
	std::unique_ptr<unsigned char> tmpBuf(new unsigned char[36]);
	unsigned char statusBuf[13];

	// first send the inquiry with the minimal allocation length
	SCSI::Inquiry inquiry1(LUN, 36, page);
	bulkTransfer(endpoint, inquiry1.getData(), inquiry1.getDataLength(), &transferred, 2000);
	bulkTransfer(inEndpoint, tmpBuf.get(), 36, &transferred, 2000);
	
	// read status
	bulkTransfer(inEndpoint, statusBuf, 13, &statusTransferred, 2000);
	
	if (transferred <= 36) {
		// TODO: throw
	}
	
	SCSI::InquiryResponse response(ByteBuffer(tmpBuf.get(), transferred));
	
	if (response.getAdditionalLength() <= 31) {
		return response;
	}
	
	// send inquiry with the correct allocation length
	uint16_t inquiryLength(5 + response.getAdditionalLength());
	tmpBuf.reset(new unsigned char[inquiryLength]);
	SCSI::Inquiry inquiry2(LUN, inquiryLength, page);
	bulkTransfer(endpoint, inquiry2.getData(), inquiry2.getDataLength(), &transferred, 2000);
	bulkTransfer(inEndpoint, tmpBuf.get(), inquiryLength, &transferred, 2000);
	
	// read status
	bulkTransfer(inEndpoint, statusBuf, 13, &statusTransferred, 2000);
	
	if (transferred <= 36) {
		// return the previous result, to have at least something
		return response;
	}
	
	response = std::move(ByteBuffer(tmpBuf.get(), transferred));
	
	return response;
}

}
}
