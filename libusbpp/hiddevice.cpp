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

#include "hiddevice.h"

#include "buffer.h"

#include <libusb.h>
#include <memory>

namespace Usbpp {
namespace HID {

HIDDevice::HIDDevice() {

}

HIDDevice::HIDDevice(const Device& device): Device(device) {

}

HIDDevice::HIDDevice(const HIDDevice& device) : Device(device) {

}

HIDDevice::HIDDevice(HIDDevice&& device) noexcept: Device(std::move(device)) {

}

ReportTree HIDDevice::getHidReport(int bInterfaceNumber) const {
	ByteBuffer tmpBuf(4096);
	int res(controlTransferIn(LIBUSB_ENDPOINT_IN | LIBUSB_RECIPIENT_INTERFACE,
	                  LIBUSB_REQUEST_GET_DESCRIPTOR,
	                  (LIBUSB_DT_REPORT << 8)|bInterfaceNumber,
	                  0,
	                  tmpBuf, 2000));
	tmpBuf.resize(res);
	return ReportTree(tmpBuf);
}

}
}
