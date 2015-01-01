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

#include "csw.h"

#include "buffer.h"

#include <cassert>
#include <cstring>

namespace Usbpp {
namespace MassStorage {

CommandStatusWrapper::CommandStatusWrapper() {
	data = new uint8_t[CSW_LEN];
	std::memset(data, 0, CSW_LEN);
}

CommandStatusWrapper::CommandStatusWrapper(const ByteBuffer &buffer) {
	assert(buffer.size() == CSW_LEN);
	
	data = new uint8_t[CSW_LEN];
	std::memcpy(data, buffer.data(), buffer.size());
}

CommandStatusWrapper::CommandStatusWrapper(uint32_t dCSWTag, uint32_t dCSWDataResidue, uint8_t bCSWStatus)
{
	data = new uint8_t[CSW_LEN];
	std::memset(data, 0, CSW_LEN);
	
	// dCSWSignature
	data[0] = 'U';
	data[1] = 'S';
	data[2] = 'B';
	data[3] = 'S';
	// dCSWTag
	data[4] = (dCSWTag >> 24) & 0xFF;
	data[5] = (dCSWTag >> 16) & 0xFF;
	data[6] = (dCSWTag >> 8) & 0xFF;
	data[7] = dCSWTag & 0xFF;
	// dCSWDataResidue
	data[8] = (dCSWDataResidue >> 24) & 0xFF;
	data[9] = (dCSWDataResidue >> 16) & 0xFF;
	data[10] = (dCSWDataResidue >> 8) & 0xFF;
	data[11] = dCSWDataResidue & 0xFF;
	// bCSWStatus
	assert(bCSWStatus != 0x03 && bCSWStatus != 0x04); // obsolete
	assert(bCSWStatus < 0x05); // reserved
	data[12] = bCSWStatus;
}

CommandStatusWrapper::~CommandStatusWrapper()
{
	// required only because of the move constructor
	if (data) {
		delete[] data;
		data = nullptr;
	}
}

CommandStatusWrapper::CommandStatusWrapper(const CommandStatusWrapper& other)
{
	data = new uint8_t[CSW_LEN];
	std::memcpy(data, other.data, CSW_LEN);
}

CommandStatusWrapper::CommandStatusWrapper(CommandStatusWrapper&& other)
{
	data = other.data;
	other.data = nullptr;
}

CommandStatusWrapper& CommandStatusWrapper::operator=(const CommandStatusWrapper& other)
{
	std::memcpy(data, other.data, CSW_LEN);
}

CommandStatusWrapper& CommandStatusWrapper::operator=(CommandStatusWrapper&& other)
{
	if (this == &other) {
		return *this;
	}
	std::memcpy(data, other.data, CSW_LEN);
	
	return *this;
}

uint32_t CommandStatusWrapper::getTag() const
{
	uint32_t tag;
	tag = data[4];
	tag = (tag << 8) | data[5];
	tag = (tag << 8) | data[6];
	tag = (tag << 8) | data[7];
	return tag;
}

uint32_t CommandStatusWrapper::getDataResidue() const
{
	uint32_t residue;
	residue = data[8];
	residue = (residue << 8) | data[9];
	residue = (residue << 8) | data[10];
	residue = (residue << 8) | data[11];
	return residue;
}

CommandStatusWrapper::Status CommandStatusWrapper::getStatus() const
{
	if (data[12] > 0x05) {
		return Status::RESERVED;
	}
	if (data[12] >= 0x03) {
		return Status::OBSOLETE;
	}
	return static_cast<Status>(data[12]);
}

std::size_t CommandStatusWrapper::getDataLength() const
{
	return CSW_LEN;
}

unsigned char* CommandStatusWrapper::getData() const
{
	return static_cast<unsigned char*>(const_cast<uint8_t*>(data));
}

ByteBuffer CommandStatusWrapper::getBuffer() const
{
	return ByteBuffer(getData(), getDataLength());
}

std::ostream& operator<<(std::ostream& os, const CommandStatusWrapper::Status &status)
{
	switch (status) {
		case Usbpp::MassStorage::CommandStatusWrapper::Status::PASSED:
			return os << "PASSED (0x00)";
		case Usbpp::MassStorage::CommandStatusWrapper::Status::FAILED:
			return os << "FAILED (0x01)";
		case Usbpp::MassStorage::CommandStatusWrapper::Status::PHASE_ERROR:
			return os << "PHASE ERROR (0x02)";
		case Usbpp::MassStorage::CommandStatusWrapper::Status::OBSOLETE:
			return os << "OBSOLETE";
		case Usbpp::MassStorage::CommandStatusWrapper::Status::RESERVED:
			return os << "RESERVED";
	}
}

}
}
