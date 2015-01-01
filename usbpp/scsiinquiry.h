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

#ifndef LIBUSBPP_MASS_SCSI_INQUIRY_H_
#define LIBUSBPP_MASS_SCSI_INQUIRY_H_

#include "cbw.h"

namespace Usbpp {
namespace MassStorage {
namespace SCSI {

class Inquiry : public CommandBlockWrapper {
public:
	/**
	 * \param allocationLength the allocation length should be at least 36
	 *                         Note that the 36 bytes is minimum for the Inquiry
	 *                         response. It may be necessary to increase the allocationLength
	 *                         based on the additional length field from response.
	 */
	Inquiry(uint8_t LUN, uint16_t allocationLength);
	Inquiry(uint8_t LUN, uint16_t allocationLength, uint8_t page);
    Inquiry(const CommandBlockWrapper& other);
    Inquiry(CommandBlockWrapper&& other);
	Inquiry& operator=(Inquiry &&other) noexcept;
};

}
}
}

#endif
