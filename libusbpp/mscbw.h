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

#ifndef LIBUSBPP_MASS_CBW_H_
#define LIBUSBPP_MASS_CBW_H_

#include "buffer.h"

#include <cstdint>
#include <vector>
#include <ostream>

namespace Usbpp {
namespace MassStorage {

class CommandBlockWrapper {
public:
	enum class Flags : uint8_t {
		DATA_OUT = 0x0,
		DATA_IN = 0x80,
		INVALID = 0x7F // either obsolete or reserved
	};
	
	CommandBlockWrapper();
	CommandBlockWrapper(const ByteBuffer &buffer);
	CommandBlockWrapper(uint32_t dCBWDataTransferLength, uint8_t bmCBWFlags, uint8_t bCBWLUN, std::vector< uint8_t > CBWCB);
	virtual ~CommandBlockWrapper();
	
	CommandBlockWrapper(const CommandBlockWrapper &other);
	CommandBlockWrapper(CommandBlockWrapper &&other) noexcept;
	CommandBlockWrapper &operator=(const CommandBlockWrapper &other);
	CommandBlockWrapper &operator=(CommandBlockWrapper &&other) noexcept;
	
	uint32_t getTag() const;
	uint32_t getTransferLength() const;
	Flags getFlags() const;
	
	/**
	 * Get the device Logical Unit Number (LUN) to which the command
	 * block is being sent.
	 * 
	 * This function doesn't check whether the returned LUN is valid.
	 */
	uint8_t getLun() const;
	uint8_t getCommandBlockLength() const;
	std::vector<uint8_t> getCommandBlock() const;
	
	const ByteBuffer &getBuffer() const;
	
protected:
	virtual uint32_t generateTag() const;
	ByteBuffer mdata;
};

std::ostream &operator<<(std::ostream &os, const CommandBlockWrapper::Flags &flags);

}
}

#endif
