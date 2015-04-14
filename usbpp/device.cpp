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

#include "device.h"

#include <cassert>
#include <libusb.h>

#include <unistd.h>

namespace Usbpp {

DeviceException::DeviceException() noexcept
{

}

DeviceException::~DeviceException()
{

}

const char* DeviceException::what() const noexcept
{
    return "A DeviceException occurred";
}

Device::Device() : device(nullptr), handle(nullptr), handleRefCount(nullptr), interfaceRefCount(nullptr)
{
	
}

Device::Device(libusb_device* device_) : device(device_), handle(nullptr), handleRefCount(nullptr), interfaceRefCount(nullptr)
{

}

Device::Device(const Device& other) : device(other.device)
{
	libusb_ref_device(device);
	handle = other.handle;
	handleRefCount = other.handleRefCount;
	if (handleRefCount) {
		++(*handleRefCount);
	}
	interfaceRefCount = other.interfaceRefCount;
}

Device::Device(Device&& other)
{
	device = other.device;
	other.device = nullptr;
	handle = other.handle;
	other.handle = nullptr;
	handleRefCount = other.handleRefCount;
	other.handleRefCount = nullptr;
	interfaceMyClaimed = std::move(other.interfaceMyClaimed);
	interfaceRefCount = other.interfaceRefCount;
	other.interfaceRefCount = nullptr;
}

Device::~Device()
{
	close();
	freeDevice();
}

bool Device::isValid()
{
	return device != nullptr;
}

Device& Device::operator=(const Device& other)
{
	if (this == &other) {
		return *this;
	}
	
	// cleanup the current device contents
	close();
	freeDevice();
	
	// do the assignment
	device = other.device;
	libusb_ref_device(device);
	handle = other.handle;
	handleRefCount = other.handleRefCount;
	if (handleRefCount) {
		++(*handleRefCount);
	}
	
	return *this;
}

Device& Device::operator=(Device&& other)
{
	if (this == &other) {
		return *this;
	}
	
	std::swap(device, other.device);
	std::swap(handle, other.handle);
	std::swap(handleRefCount, other.handleRefCount);
	std::swap(interfaceMyClaimed, other.interfaceMyClaimed);
	std::swap(interfaceRefCount, other.interfaceRefCount);
	
	return *this;
}

void Device::open(bool detachDriver)
{
	if (! handleRefCount) {
		if (libusb_open(device, &handle) != 0) {
			throw DeviceException();
		}
		handleRefCount = new int;
		*handleRefCount = 1;
	}
	libusb_set_auto_detach_kernel_driver(handle, detachDriver);
}

void Device::close()
{
	// release interfaces claimed by this object
	while (interfaceMyClaimed.begin() != interfaceMyClaimed.end()) {
		releaseInterface(* interfaceMyClaimed.begin());
	}
	// close the device
	if (handleRefCount) {
		--(*handleRefCount);
		if (*handleRefCount == 0) {
			libusb_close(handle);
			handle = nullptr;
			delete handleRefCount;
			handleRefCount = nullptr;
		}
	}
}

bool Device::reset()
{
	assert(handle != 0);
	assert(interfaceMyClaimed.empty());
	assert(interfaceRefCount == 0 || interfaceRefCount->empty());
	if (libusb_reset_device(handle) == LIBUSB_ERROR_NOT_FOUND) {
		return false;
	}
	return true;
}

void Device::clearHalt(unsigned char endpoint)
{
	libusb_clear_halt(handle, endpoint);
}

libusb_device_descriptor Device::getDescriptor()
{
	libusb_device_descriptor desc;
	libusb_get_device_descriptor(device, &desc);
	return desc;
}

int Device::getConfiguration()
{
	int config;
	libusb_get_configuration(handle, &config);
	return config;
}

void Device::setConfiguration(int bConfigurationValue)
{
	libusb_set_configuration(handle, bConfigurationValue);
}

void Device::claimInterface(int bInterfaceNumber)
{
	interfaceMyClaimed.emplace(bInterfaceNumber);
	if (! interfaceRefCount) {
		interfaceRefCount = new std::unordered_map<int, int>();
	}
	std::unordered_map<int, int>::iterator refcnt(interfaceRefCount->find(bInterfaceNumber));
	if (refcnt == interfaceRefCount->end()) {
		interfaceRefCount->insert(std::make_pair(bInterfaceNumber, 1));
	}
	else {
		++(refcnt->second);
	}
	libusb_claim_interface(handle, bInterfaceNumber);
}

void Device::releaseInterface(int bInterfaceNumber)
{
	std::unordered_set<int>::iterator it(interfaceMyClaimed.find(bInterfaceNumber));
	if (it != interfaceMyClaimed.end()) {
		interfaceMyClaimed.erase(it);
		std::unordered_map<int, int>::iterator refcnt(interfaceRefCount->find(bInterfaceNumber));
		--(refcnt->second);
		if (refcnt->second == 0) {
			libusb_release_interface(handle, bInterfaceNumber);
			interfaceRefCount->erase(refcnt);
		}
		if (interfaceRefCount->empty()) {
			delete interfaceRefCount;
			interfaceRefCount = nullptr;
		}
	}
}

void Device::controlTransfer(uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char* data, uint16_t wLength, unsigned int timeout) const
{
	libusb_control_transfer(handle, bmRequestType, bRequest, wValue, wIndex, data, wLength, timeout);
}

void Device::bulkTransfer(unsigned char endpoint, unsigned char* data, int length, int* transferred, unsigned int timeout) const
{
	libusb_bulk_transfer(handle, endpoint, data, length, transferred, timeout);
}

void Device::interruptTransfer(unsigned char endpoint, unsigned char* data, int length, int* transferred, unsigned int timeout) const
{
	libusb_interrupt_transfer(handle, endpoint, data, length, transferred, timeout);
}

void Device::freeDevice()
{
	if (device) {
		libusb_unref_device(device);
		device = nullptr;
	}
}

}
