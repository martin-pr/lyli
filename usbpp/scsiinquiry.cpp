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

#include "scsiinquiry.h"

#include <cassert>

namespace Usbpp {
namespace MassStorage {
namespace SCSI {

Inquiry::Inquiry(uint8_t LUN, uint16_t allocationLength) :
	CommandBlockWrapper(allocationLength, 0x80, LUN,
	                    {0x12,
	                     0,
	                     0,
	                     static_cast<uint8_t>((allocationLength >> 8) & 0xFF), static_cast<uint8_t>(allocationLength & 0xFF),
	                     0})
{
	assert(allocationLength >= 36);
}

Inquiry::Inquiry(uint8_t LUN, uint16_t allocationLength, uint8_t page) :
	CommandBlockWrapper(allocationLength, 0x80, LUN,
	                    {0x12,
	                     0x01,
	                     page,
	                     static_cast<uint8_t>((allocationLength >> 8) & 0xFF), static_cast<uint8_t>(allocationLength & 0xFF),
	                     0})
{
	assert(allocationLength >= 36);
}

Inquiry::Inquiry(const CommandBlockWrapper& other): CommandBlockWrapper(other)
{

}

Inquiry::Inquiry(CommandBlockWrapper&& other): CommandBlockWrapper(std::move(other))
{

}

}
}
}
