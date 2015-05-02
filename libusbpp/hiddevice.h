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

#ifndef LIBUSBPP_HID_DEVICE_H_
#define LIBUSBPP_HID_DEVICE_H_

#include "device.h"
#include "hidreport.h"

namespace Usbpp {
namespace HID {

/**
 * USB HID device.
 *
 * The HID device can be constructed from a standard Device class.
 * It provides an easy interface to HID specific features.
 */
class HIDDevice : public Device {
public:
	using Device::Device;

	HIDDevice();
	HIDDevice(const Device &device);
	HIDDevice(const HIDDevice &device);
	HIDDevice(HIDDevice &&device) noexcept;
	
	HIDDevice& operator=(const HIDDevice &other) = default;
	HIDDevice& operator=(HIDDevice &&other) noexcept = default;
	
	/**
	 * Get the USB HID report descriptor.
	 *
	 * @param bInterfaceNumber interface to use.
	 * @return report descriptor parsed as a tree.
	 */
	ReportTree getHidReport(int bInterfaceNumber) const;
};

}
}

#endif
