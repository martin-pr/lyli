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

#ifndef LIBUSBPP_DEVICE_H_
#define LIBUSBPP_DEVICE_H_

#include <exception>
#include <memory>
#include <string>

#include "buffer.h"

class libusb_device_descriptor;
class libusb_device;

namespace Usbpp {

class Context;

/**
 * An exception thrown when an error occurs within device class.
 */
class DeviceException {
public:
	DeviceException(int error) noexcept;
	virtual ~DeviceException();

	int getError() const;

	virtual const char* what() const noexcept = 0;
private:
	const int error;
};

/**
 * An exception when the device cannot be opened.
 */
class DeviceOpenException : public DeviceException {
public:
	DeviceOpenException(int error) noexcept;
	virtual ~DeviceOpenException();

	virtual const char* what() const noexcept;
};

/**
 * An exception when the transfer failed.
 */
class DeviceTransferException : public DeviceException {
public:
	DeviceTransferException(int error) noexcept;
	virtual ~DeviceTransferException();

	virtual const char* what() const noexcept;
};

/**
 * An USB device.
 *
 * This class provides an interface to communicate with the connected USB device.
 */
class Device {
public:
	friend class Context;

	/**
	 * Default constructor required for use within standard containers.
	 *
	 * It should be avoided as the device contructed with the default constructor is
	 * not a valid device.
	 */
	Device();
	Device(const Device &other);
	Device(Device &&other) noexcept;
	~Device();

	Device& operator=(const Device &other);
	Device& operator=(Device &&other) noexcept;

	/**
	 * Check whether the device is valid USB device.
	 *
	 * \return true if the device is valid.
	 */
	bool isValid();

	/**
	 * Open the device for use.
	 *
	 * \param detachDriver auto detach kernel driver if the kernel driver is active
	 * when claiming the interface and reattach it when the interface is released.
	 */
	void open(bool detachDriver);
	/**
	 * Close the device.
	 *
	 * The device is closed only after all the Device instances that share
	 * the same device are closed.
	 *
	 * All currently claimed interfaces are released before closing the device.
	 * Note that only the interfaces claimed by the current instance so we don't
	 * unexpectedly release an interface claimed by another object sharing the
	 * same device.
	 */
	void close();

	/**
	 *  Reset the device.
	 *
	 * The reset can only be performed is the device has not been claimed.
	 *
	 * \return true if the device has been reset. If false is returned, the device
	 * has been disconnected & connected, meaning that the device must be rediscovered
	 * and reopened again.
	 */
	bool reset();

	/**
	 * Clear halt on an endpoint.
	 */
	void clearHalt(unsigned char endpoint);

	/**
	 * Get the device descriptor.
	 *
	 * @return a libusb_device_descriptor structure containing the device descriptor.
	 *         The structure is defined in libusb.h
	 */
	libusb_device_descriptor getDescriptor();

	/**
	 * Get the device configuration.
	 *
	 * @return configuration.
	 */
	int getConfiguration();

	/**
	 * Set the device configuration.
	 *
	 * @param bConfigurationValue configuration
	 */
	void setConfiguration(int bConfigurationValue);

	/**
	 * Claim an interface for use.
	 *
	 * @param bInterfaceNumber interface to claim
	 */
	void claimInterface(int bInterfaceNumber);
	/**
	 * Release the interface.
	 *
	 * The interface is released only after all Devices that claimed this
	 * interface release it.
	 *
	 * @param bInterfaceNumber interface to release
	 */
	void releaseInterface(int bInterfaceNumber);

	/**
	 * Control transfer from the device to the computer ("receive").
	 *
	 * @return number of bytes actually transferred.
	 */
	int controlTransferIn(uint8_t bmRequestType,
	                       uint8_t bRequest,
	                       uint16_t wValue,
	                       uint16_t wIndex,
	                       ByteBuffer &data,
	                       unsigned int timeout) const;
	/**
	 * Bulk transfer from the device to the computer ("receive").
	 *
	 * @return number of bytes actually transferred.
	 */
	int bulkTransferIn(unsigned char endpoint,
	                   ByteBuffer &data,
	                   unsigned int timeout) const;
	/**
	 * Interrupt transfer from the device to the computer ("receive").
	 *
	 * @return number of bytes actually transferred.
	 */
	int interruptTransferIn(unsigned char endpoint,
	                        ByteBuffer &data,
	                        unsigned int timeout) const;

	/**
	 * Control transfer from computer to device ("send").
	 *
	 * @return number of bytes actually transferred.
	 */
	int controlTransferOut(uint8_t bmRequestType,
	                        uint8_t bRequest,
	                        uint16_t wValue,
	                        uint16_t wIndex,
	                        const ByteBuffer &data,
	                        unsigned int timeout) const;
	/**
	 * Bulk transfer from computer to device ("send").
	 *
	 * @return number of bytes actually transferred.
	 */
	int bulkTransferOut(unsigned char endpoint,
	                    const ByteBuffer &data,
	                    unsigned int timeout) const;
	/**
	 * Interrupt transfer from computer to device ("send").
	 *
	 * @return number of bytes actually transferred.
	 */
	int interruptTransferOut(unsigned char endpoint,
	                         const ByteBuffer &data,
	                         unsigned int timeout) const;

private:
	Device(libusb_device *device_);
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

}

#endif


