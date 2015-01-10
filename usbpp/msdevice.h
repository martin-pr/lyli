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

#ifndef LIBUSBPP_MASS_DEBICE_H_
#define LIBUSBPP_MASS_DEBICE_H_

#include "buffer.h"
#include "device.h"

namespace Usbpp {
namespace MassStorage {

class CommandStatusWrapper;
class CommandBlockWrapper;

namespace SCSI {
class InquiryResponse;
}

class MSDevice : public Device {
public:
	using Device::Device;
	
    MSDevice(const Device &device);
	MSDevice(const MSDevice &device);
	MSDevice(MSDevice &&device);
	
	CommandStatusWrapper sendCommand(unsigned char endpoint, const CommandBlockWrapper& command, ByteBuffer *data);
	
	/**
	 * A convenience function to send the SCSI Inquiry command.
	 */
	SCSI::InquiryResponse sendInquiry(unsigned char endpoint, uint8_t LUN) const;
	/**
	 * A convenience function to send the SCSI Inquiry command.
	 */
	SCSI::InquiryResponse sendInquiry(unsigned char endpoint, uint8_t LUN, uint8_t page) const;
};

}
}

#endif
