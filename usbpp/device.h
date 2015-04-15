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

#include <libusb.h>
#include <unordered_map>
#include <unordered_set>
#include <exception>

namespace Usbpp {

class Context;

class DeviceException : public std::exception {
public:
	DeviceException() noexcept;
    virtual ~DeviceException();
	
	virtual const char* what() const noexcept;
};

class Device {
public:
	friend class Context;
	
	Device();
	Device(const Device &other);
	Device(Device &&other) noexcept;
	~Device();
	
	Device& operator=(const Device &other);
	Device& operator=(Device &&other) noexcept;
	
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
	
	void clearHalt(unsigned char endpoint);
	
	libusb_device_descriptor getDescriptor();
	
	int getConfiguration();
	
	void setConfiguration(int bConfigurationValue);
	
	/**
	 * Claim an interface for use.
	 */
	void claimInterface(int bInterfaceNumber);
	/**
	 * Release the interface.
	 * 
	 * The interface is released only after all Devices that claimed this
	 * interface release it.
	 */
	void releaseInterface(int bInterfaceNumber);
	
	void controlTransfer(uint8_t bmRequestType,
	                     uint8_t bRequest,
	                     uint16_t wValue,
	                     uint16_t wIndex,
	                     unsigned char *data,
	                     uint16_t wLength,
	                     unsigned int timeout) const;
	void bulkTransfer(unsigned char endpoint,
	                  unsigned char *data,
	                  int length,
	                  int *transferred,
	                  unsigned int timeout) const;
	void interruptTransfer(unsigned char endpoint,
	                       unsigned char *data,
	                       int length,
	                       int *transferred,
	                       unsigned int timeout) const;
	
private:
	Device(libusb_device *device_);
	void freeDevice();
	
	libusb_device *device;
	libusb_device_handle *handle;
	int *handleRefCount;
	// a set of interfaces claimed by the device
	std::unordered_set<int> interfaceMyClaimed;
	// a shared map storing the reference counts for all interfaces
	std::unordered_map<int, int> *interfaceRefCount;
};

}

#endif
