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

#include "mscbw.h"

#include <cassert>
#include <cstring>

namespace {
constexpr std::size_t CBW_LEN = 31;
}

namespace Usbpp {
namespace MassStorage {

CommandBlockWrapper::CommandBlockWrapper() : mdata(CBW_LEN) {
	std::memset(&mdata[0], 0, CBW_LEN);
}

CommandBlockWrapper::CommandBlockWrapper(const ByteBuffer &buffer) {
	assert(buffer.size() <= CBW_LEN);

	mdata = buffer;
	mdata.resize(CBW_LEN);
	// fill the rest with zeroes
	if(buffer.size() != CBW_LEN) {
		std::memset(&mdata[buffer.size()], 0, CBW_LEN - buffer.size());
	}
}

CommandBlockWrapper::CommandBlockWrapper(uint32_t dCBWDataTransferLength,
                                         uint8_t bmCBWFlags,
                                         uint8_t bCBWLUN,
                                         std::vector<uint8_t> CBWCB) :
	mdata(CBW_LEN)
{
	// only CBWCB needs to be zeroed as the rest will be written into
	std::memset(&mdata[15], 0, CBW_LEN - 15);
	
	// dCBWSignature
	mdata[0] = 'U';
	mdata[1] = 'S';
	mdata[2] = 'B';
	mdata[3] = 'C';
	// dCBWTag
	uint32_t tag(generateTag());
	mdata[4] = tag & 0xFF;
	mdata[5] = (tag >> 8) & 0xFF;
	mdata[6] = (tag >> 16) & 0xFF;
	mdata[7] = (tag >> 24) & 0xFF;
	// dCBWDataTransferLength
	mdata[8] = dCBWDataTransferLength & 0xFF;
	mdata[9] = (dCBWDataTransferLength >> 8) & 0xFF;
	mdata[10] = (dCBWDataTransferLength >> 16) & 0xFF;
	mdata[11] = (dCBWDataTransferLength >> 24) & 0xFF;
	// bmCBWFlags
	assert((bmCBWFlags & 0x3F) ==  0); // reserved bits
	assert((bmCBWFlags & 0x40) ==  0); // obsolete bits
	mdata[12] = bmCBWFlags;
	// bCBWLUN
	assert((bCBWLUN & 0xF) ==  bCBWLUN);
	mdata[13] = bCBWLUN;
	// bCBWCBLength
	assert((CBWCB.size() & 0x1F) ==  CBWCB.size());
	mdata[14] = CBWCB.size() & 0x1F;
	// CBWCB
	std::memcpy(&mdata[15], &CBWCB[0], CBWCB.size());
}

CommandBlockWrapper::~CommandBlockWrapper() {

}

CommandBlockWrapper::CommandBlockWrapper(const CommandBlockWrapper& other) : mdata(other.mdata) {

}

CommandBlockWrapper::CommandBlockWrapper(CommandBlockWrapper&& other) noexcept : mdata(std::move(other.mdata)) {

}

CommandBlockWrapper& CommandBlockWrapper::operator=(const CommandBlockWrapper& other)
{
	if (this == &other) {
		return *this;
	}
	mdata = other.mdata;
	
	return *this;
}

CommandBlockWrapper& CommandBlockWrapper::operator=(CommandBlockWrapper&& other) noexcept
{
	if (this == &other) {
		return *this;
	}
	
	std::swap(mdata, other.mdata);
	
	return *this;
}

uint32_t CommandBlockWrapper::getTag() const
{
	uint32_t tag;
	tag = mdata[7];
	tag = (tag << 8) | mdata[6];
	tag = (tag << 8) | mdata[5];
	tag = (tag << 8) | mdata[4];
	return tag;
}

uint32_t CommandBlockWrapper::getTransferLength() const
{
	uint32_t len;
	len = mdata[11];
	len = (len << 8) | mdata[10];
	len = (len << 8) | mdata[9];
	len = (len << 8) | mdata[8];
	return len;
}

CommandBlockWrapper::Flags CommandBlockWrapper::getFlags() const
{
	if ((mdata[12] & static_cast<uint8_t>(Flags::INVALID)) != 0) {
		return Flags::INVALID;
	}
	return static_cast<Flags>(mdata[12]);
}

uint8_t CommandBlockWrapper::getLun() const
{
	return mdata[13];
}

uint8_t CommandBlockWrapper::getCommandBlockLength() const
{
	return mdata[14];
}

std::vector< uint8_t > CommandBlockWrapper::getCommandBlock() const
{
	return std::vector<uint8_t>(&mdata[15], &mdata[15] + getCommandBlockLength());
}

const ByteBuffer &CommandBlockWrapper::getBuffer() const
{
	return mdata;
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
