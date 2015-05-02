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
#include <unordered_map>
#include <unordered_set>
#include <sstream>

namespace Usbpp {

DeviceException::DeviceException(int error_) noexcept : error(error_) {

}

DeviceException::~DeviceException() {

}

int DeviceException::getError() const {
	return error;
}

DeviceOpenException::DeviceOpenException(int error) noexcept : DeviceException(error) {

}

DeviceOpenException::~DeviceOpenException() {

}

const char* DeviceOpenException::what() const noexcept {
	return "Cannot open the device";
}

DeviceTransferException::DeviceTransferException(int error) noexcept : DeviceException(error) {

}

DeviceTransferException::~DeviceTransferException() {

}

const char* DeviceTransferException::what() const noexcept {
	return "Transfer failed";
}

class Device::Impl {
public:
	Impl();
	Impl(libusb_device *device_);
	Impl(const Impl& other);
	~Impl();

	void close();
	void releaseInterface(int bInterfaceNumber);

	libusb_device *device;
	libusb_device_handle *handle;
	int *handleRefCount;
	// a set of interfaces claimed by the device
	std::unordered_set<int> interfaceMyClaimed;
	// a shared map storing the reference counts for all interfaces
	std::unordered_map<int, int> *interfaceRefCount;
};

Device::Impl::Impl() :
	device(nullptr),
	handle(nullptr),
	handleRefCount(nullptr),
	interfaceRefCount(nullptr) {

}

Device::Impl::Impl(libusb_device* device_) :
	device(device_),
	handle(nullptr),
	handleRefCount(nullptr),
	interfaceRefCount(nullptr) {

}

Device::Impl::Impl(const Impl& other) :
	device(other.device),
	handle(other.handle),
	handleRefCount(other.handleRefCount),
	interfaceRefCount(other.interfaceRefCount) {

	if(device) {
		libusb_ref_device(device);
	}
	if (handleRefCount) {
		++(*handleRefCount);
	}
}

Device::Impl::~Impl() {
	// close handle (decreases the handle and interface refcounts, too)
	close();
	// decrease the device refcount
	if (device) {
		libusb_unref_device(device);
		device = nullptr;
	}
}

void Device::Impl::close() {
	// release interfaces claimed by this object
	while (interfaceMyClaimed.begin() != interfaceMyClaimed.end()) {
		releaseInterface(*interfaceMyClaimed.begin());
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

void Device::Impl::releaseInterface(int bInterfaceNumber) {
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

Device::Device() : pimpl(new Impl) {

}

Device::Device(const Device &other) : pimpl(new Impl(*other.pimpl)) {

}

Device::Device(Device &&other) noexcept : pimpl(std::move(other.pimpl)) {

}

Device::Device(libusb_device* device_) : pimpl(new Impl(device_)) {

}

Device::~Device() {

}

bool Device::isValid() {
	return pimpl->device != nullptr;
}

Device& Device::operator=(const Device& other) {
	if (this == &other) {
		return *this;
	}

	Device tmp(other);
	std::swap(pimpl, tmp.pimpl);

	return *this;
}

Device& Device::operator=(Device && other) noexcept {
	if (this == &other) {
		return *this;
	}

	std::swap(pimpl, other.pimpl);

	return *this;
}

void Device::open(bool detachDriver) {
	if (! pimpl->handleRefCount) {
		int res(libusb_open(pimpl->device, &pimpl->handle));
		if (res != 0) {
			throw DeviceOpenException(res);
		}
		pimpl->handleRefCount = new int;
		*pimpl->handleRefCount = 1;
	}
	libusb_set_auto_detach_kernel_driver(pimpl->handle, detachDriver);
}

void Device::close() {
	pimpl->close();
}

bool Device::reset() {
	assert(pimpl->handle != 0);
	assert(pimpl->interfaceMyClaimed.empty());
	assert(pimpl->interfaceRefCount == 0 || pimpl->interfaceRefCount->empty());
	if (libusb_reset_device(pimpl->handle) == LIBUSB_ERROR_NOT_FOUND) {
		return false;
	}
	return true;
}

void Device::clearHalt(unsigned char endpoint) {
	libusb_clear_halt(pimpl->handle, endpoint);
}

libusb_device_descriptor Device::getDescriptor() {
	libusb_device_descriptor desc;
	libusb_get_device_descriptor(pimpl->device, &desc);
	return desc;
}

int Device::getConfiguration() {
	int config;
	libusb_get_configuration(pimpl->handle, &config);
	return config;
}

void Device::setConfiguration(int bConfigurationValue) {
	libusb_set_configuration(pimpl->handle, bConfigurationValue);
}

void Device::claimInterface(int bInterfaceNumber) {
	pimpl->interfaceMyClaimed.emplace(bInterfaceNumber);
	if (! pimpl->interfaceRefCount) {
		pimpl->interfaceRefCount = new std::unordered_map<int, int>();
	}
	std::unordered_map<int, int>::iterator refcnt(pimpl->interfaceRefCount->find(bInterfaceNumber));
	if (refcnt == pimpl->interfaceRefCount->end()) {
		pimpl->interfaceRefCount->insert(std::make_pair(bInterfaceNumber, 1));
	}
	else {
		++(refcnt->second);
	}
	libusb_claim_interface(pimpl->handle, bInterfaceNumber);
}

void Device::releaseInterface(int bInterfaceNumber) {
	pimpl->releaseInterface(bInterfaceNumber);
}

int Device::controlTransferIn(uint8_t bmRequestType,
                               uint8_t bRequest,
                               uint16_t wValue,
                               uint16_t wIndex,
                               ByteBuffer &data,
                               unsigned int timeout) const {
	assert(bmRequestType & LIBUSB_ENDPOINT_IN);
	int res(libusb_control_transfer(pimpl->handle, bmRequestType, bRequest, wValue, wIndex, data.data(), data.size(), timeout));
	if (res < 0) {
		throw DeviceTransferException(res);
	}
	return res;
}

int Device::bulkTransferIn(unsigned char endpoint,
                            ByteBuffer &data,
                            unsigned int timeout) const {
	assert(endpoint & LIBUSB_ENDPOINT_IN);
	int transferred(0);
	int res(libusb_bulk_transfer(pimpl->handle, endpoint, data.data(), data.size(), &transferred, timeout));
	if (res != 0) {
		throw DeviceTransferException(res);
	}
	return transferred;
}

int Device::interruptTransferIn(unsigned char endpoint,
                                ByteBuffer &data,
                                unsigned int timeout) const {
	assert(endpoint & LIBUSB_ENDPOINT_IN);
	int transferred(0);
	int res(libusb_interrupt_transfer(pimpl->handle, endpoint, data.data(), data.size(), &transferred, timeout));
	if (res != 0) {
		throw DeviceTransferException(res);
	}
	return transferred;
}

int Device::controlTransferOut(uint8_t bmRequestType,
                                uint8_t bRequest,
                                uint16_t wValue,
                                uint16_t wIndex,
                                const ByteBuffer &data,
                                unsigned int timeout) const {
	assert((bmRequestType & LIBUSB_ENDPOINT_IN) == 0);
	int res(libusb_control_transfer(pimpl->handle, bmRequestType, bRequest, wValue, wIndex,
	                                const_cast<unsigned char*>(data.data()), data.size(), timeout));
	if (res < 0) {
		throw DeviceTransferException(res);
	}
	return res;
}

int Device::bulkTransferOut(unsigned char endpoint,
                            const ByteBuffer &data,
                            unsigned int timeout) const {
	assert((endpoint & LIBUSB_ENDPOINT_IN) == 0);
	int transferred(0);
	int res(libusb_bulk_transfer(pimpl->handle, endpoint, const_cast<unsigned char*>(data.data()), data.size(), &transferred, timeout));
	if (res != 0) {
		throw DeviceTransferException(res);
	}
	return transferred;
}

int Device::interruptTransferOut(unsigned char endpoint,
                                 const ByteBuffer &data,
                                 unsigned int timeout) const {
	assert((endpoint & LIBUSB_ENDPOINT_IN) == 0);
	int transferred(0);
	int res(libusb_interrupt_transfer(pimpl->handle, endpoint, const_cast<unsigned char*>(data.data()), data.size(), &transferred, timeout));
	if (res != 0) {
		throw DeviceTransferException(res);
	}
	return transferred;
}

}
