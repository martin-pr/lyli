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

#include "cbw.h"

#include <cassert>
#include <cstring>

namespace Usbpp {
namespace MassStorage {

CommandBlockWrapper::CommandBlockWrapper() {
	data = new uint8_t[CBW_LEN];
	std::memset(data, 0, CBW_LEN);
}

CommandBlockWrapper::CommandBlockWrapper(const ByteBuffer &buffer) {
	assert(buffer.size() <= CBW_LEN);
	
	data = new uint8_t[CBW_LEN];
	std::memcpy(data, buffer.data(), buffer.size());
}

CommandBlockWrapper::CommandBlockWrapper(
	uint32_t dCBWDataTransferLength,
	uint8_t bmCBWFlags,
	uint8_t bCBWLUN,
	std::vector<uint8_t> CBWCB
)
{
	data = new uint8_t[CBW_LEN];
	// only CBWCB needs to be zeroed as the rest will be written into
	std::memset(&data[15], 0, CBW_LEN - 15);
	
	// dCBWSignature
	data[0] = 'U';
	data[1] = 'S';
	data[2] = 'B';
	data[3] = 'C';
	// dCBWTag
	uint32_t tag(generateTag());
	data[4] = tag & 0xFF;
	data[5] = (tag >> 8) & 0xFF;
	data[6] = (tag >> 16) & 0xFF;
	data[7] = (tag >> 24) & 0xFF;
	// dCBWDataTransferLength
	data[8] = dCBWDataTransferLength & 0xFF;
	data[9] = (dCBWDataTransferLength >> 8) & 0xFF;
	data[10] = (dCBWDataTransferLength >> 16) & 0xFF;
	data[11] = (dCBWDataTransferLength >> 24) & 0xFF;
	// bmCBWFlags
	assert((bmCBWFlags & 0x3F) ==  0); // reserved bits
	assert((bmCBWFlags & 0x40) ==  0); // obsolete bits
	data[12] = bmCBWFlags;
	// bCBWLUN
	assert((bCBWLUN & 0xF) ==  bCBWLUN);
	data[13] = bCBWLUN;
	// bCBWCBLength
	assert((CBWCB.size() & 0x1F) ==  CBWCB.size());
	data[14] = CBWCB.size() & 0x1F;
	// CBWCB
	std::memcpy(&data[15], &CBWCB[0], CBWCB.size());
}

CommandBlockWrapper::~CommandBlockWrapper()
{
	// required only because of the move constructor
	if (data) {
		delete[] data;
		data = nullptr;
	}
}

CommandBlockWrapper::CommandBlockWrapper(const CommandBlockWrapper& other)
{
	data = new uint8_t[CBW_LEN];
	std::memcpy(data, other.data, CBW_LEN);
}

CommandBlockWrapper::CommandBlockWrapper(CommandBlockWrapper&& other) noexcept
{
	data = other.data;
	other.data = nullptr;
}

CommandBlockWrapper& CommandBlockWrapper::operator=(const CommandBlockWrapper& other)
{
	if (this == &other) {
		return *this;
	}
	std::memcpy(data, other.data, CBW_LEN);
	
	return *this;
}

CommandBlockWrapper& CommandBlockWrapper::operator=(CommandBlockWrapper&& other) noexcept
{
	if (this == &other) {
		return *this;
	}
	
	std::swap(data, other.data);
	
	return *this;
}

uint32_t CommandBlockWrapper::getTag() const
{
	uint32_t tag;
	tag = data[7];
	tag = (tag << 8) | data[6];
	tag = (tag << 8) | data[5];
	tag = (tag << 8) | data[4];
	return tag;
}

uint32_t CommandBlockWrapper::getTransferLength() const
{
	uint32_t len;
	len = data[11];
	len = (len << 8) | data[10];
	len = (len << 8) | data[9];
	len = (len << 8) | data[8];
	return len;
}

CommandBlockWrapper::Flags CommandBlockWrapper::getFlags() const
{
	if ((data[12] & static_cast<uint8_t>(Flags::INVALID)) != 0) {
		return Flags::INVALID;
	}
	return static_cast<Flags>(data[12]);
}

uint8_t CommandBlockWrapper::getLun() const
{
	return data[13];
}

uint8_t CommandBlockWrapper::getCommandBlockLength() const
{
	return data[14];
}

std::vector< uint8_t > CommandBlockWrapper::getCommandBlock() const
{
	return std::vector<uint8_t>(&data[15], &data[15] + getCommandBlockLength());
}

std::size_t CommandBlockWrapper::getDataLength() const
{
	return CBW_LEN;
}

unsigned char* CommandBlockWrapper::getData() const
{
	return static_cast<unsigned char*>(const_cast<uint8_t*>(data));
}

ByteBuffer CommandBlockWrapper::getBuffer() const
{
	return ByteBuffer(getData(), getDataLength());
}

uint32_t CommandBlockWrapper::generateTag() const
{
	static uint32_t tag;
	return tag++;
}

std::ostream& operator<<(std::ostream& os, const CommandBlockWrapper::Flags& flags)
{
	switch (flags) {
		case Usbpp::MassStorage::CommandBlockWrapper::Flags::DATA_IN:
			return os << "DATA_IN (0x00)";
		case Usbpp::MassStorage::CommandBlockWrapper::Flags::DATA_OUT:
			return os << "DATA_OUT (0x80)";
		case Usbpp::MassStorage::CommandBlockWrapper::Flags::INVALID:
			return os << "INVALID (0x7F)";
	}
	
	return os;
}

}
}
