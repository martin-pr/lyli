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

#include "context.h"

#include <cassert>
#include <libusb.h>

namespace Usbpp {

Context::Context()
{
	refcount = new int;
	*refcount = 1;
	libusb_init(&ctx);
}

Context::Context(const Context& other) : refcount(other.refcount), ctx(other.ctx)
{
	++(*refcount);
}

Context::Context(Context&& other) noexcept: refcount(other.refcount), ctx(other.ctx)
{
	other.refcount = nullptr;
	other.ctx = nullptr;
}

Context::~Context()
{
	if (refcount) {
		--(*refcount);
		if (*refcount == 0) {
			libusb_exit(ctx);
			ctx = nullptr;
			delete refcount;
			refcount = nullptr;
		}
	}
}

Context& Context::operator=(const Context& other)
{
	if (this == &other) {
		return *this;
	}
	
	// both point to the same context => do nothing
	if (refcount == other.refcount) {
		// if the refcount is the same, the context is the same, too
		assert(ctx == other.ctx);
		
		return *this;
	}
	
	// close the current context if we are the only object holding it and do the assignment
	if (*refcount == 1) {
		libusb_exit(ctx);
	}
	refcount = other.refcount;
	ctx = other.ctx;
	++(*refcount);
	
	return *this;
}

Context& Context::operator=(Context &&other) noexcept
{
	if (this == &other) {
		return *this;
	}
	
	std::swap(refcount, other.refcount);
	std::swap(ctx, other.ctx);
	
	return *this;
}

std::vector< Device > Context::getDevices()
{
	libusb_device **devices;
	int count = libusb_get_device_list(ctx, &devices);
	
	std::vector<Device> devicesRes;
	for (std::size_t i(0); i < count; ++i) {
		devicesRes.push_back(Device(devices[i]));
	}
	
	libusb_free_device_list(devices, 0);
	
	return devicesRes;
}


}
