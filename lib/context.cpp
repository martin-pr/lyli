/*
 * This file is part of Lyli, an application to control Lytro camera
 * Copyright (C) 2016  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
 * 
 * Lyli is free software: you can redistribute it and/or modify
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
 * 
 */

#include "context.h"

#include <memory>

#include <libusb.h>

#include "libusbpp/context.h"
#include "libusbpp/device.h"
#include "libusbpp/msdevice.h"

namespace {

static constexpr std::uint16_t LYTRO_VENDOR = 0x24cf;
static constexpr std::uint16_t LYTRO_PRODUCT = 0x00a1;

}

namespace Lyli {

class Context::Impl {
public:
	/**
	 * A constructor
	 */
	Impl();

	::Usbpp::Context m_context;
};

Context::Impl::Impl() {

}

Context::Context() : pimpl(new Impl) {

}

Context::~Context() {

}

CameraList Context::getCameras() {
	CameraList cameras;

	std::vector<Usbpp::Device> devices(pimpl->m_context.getDevices());

	for (Usbpp::Device dev : devices) {
		try {
			libusb_device_descriptor descr(dev.getDescriptor());
			if (descr.idVendor == LYTRO_VENDOR && descr.idProduct == LYTRO_PRODUCT) {
				cameras.push_back(Camera(dev));
			}
		}
		catch (const Usbpp::Exception &e) {
			// just silently ignore the exception and try the next device
			continue;
		}
	}

	return cameras;
}

}
