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

#ifndef LIBUSBPP_MASS_SCSI_INQUIRYRESPONSE_H_
#define LIBUSBPP_MASS_SCSI_INQUIRYRESPONSE_H_

#include "buffer.h"

namespace Usbpp {
namespace MassStorage {
namespace SCSI {

class InquiryResponse {
public:
    InquiryResponse(const ByteBuffer& buffer);
    InquiryResponse(const InquiryResponse& other);
    InquiryResponse(InquiryResponse&& other) noexcept;
	InquiryResponse& operator=(InquiryResponse &&other) noexcept;
	
	uint8_t getPeripheralQualifier() const;
	uint8_t getPeripheralDeviceType() const;
	bool getRMB() const;
	uint8_t getVersion() const;
	bool getNORMACA() const;
	bool getHISUP() const;
	uint8_t getResponseDataFormat() const;
	uint8_t getAdditionalLength() const;
	bool getSCCS() const;
	bool getACC() const;
	uint8_t getTPGS() const;
	bool get3PC() const;
	bool getProtect() const;
	bool getBQUE() const;
	bool getENCSERV() const;
	bool getMULTIP() const;
	bool getMCHNGR() const;
	bool getADDR16() const;
	bool getWBUS16() const;
	bool getSYNC() const;
	bool getLINKED() const;
	bool getCMDQUE() const;
	
	ByteBuffer getVendorIdentification() const;
	ByteBuffer getProductIdentification() const;
	ByteBuffer getProductRevisionLevel() const;
	
	/********************************
	 * fields that may not be present
	 *******************************/
	
	ByteBuffer getDriverSerialNumber() const;
	/**
	 * Get the vendor unique bits: bit 44 - 55
	 */
	ByteBuffer getVendorUnique() const;
	
	uint8_t getClocking() const;
	bool getQAS() const;
	bool getIUS() const;
	
	/**
	 * Get version descriptor.
	 * 
	 * \param descriptor a descriptor in range 0-7
	 */
	ByteBuffer getVersionDescriptor(int descriptor) const;
	
	ByteBuffer getVendorSpecific() const;

private:
	ByteBuffer mbuffer;
};

}
}
}

#endif
