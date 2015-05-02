/*
 * This file is part of Lyli, an application to control Lytro camera
 * Copyright (C) 2015  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
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
 */

#include "camera.h"

#include "filelistparser.h"

#include "libusbpp/buffer.h"
#include "libusbpp/context.h"
#include "libusbpp/msdevice.h"
#include "libusbpp/mscbw.h"
#include "libusbpp/mscsw.h"
#include "libusbpp/msscsiinquiryresponse.h"

#include <cassert>
#include <cstring>
#include <libusb.h>
#include <mutex>
#include <ostream>
#include <thread>

// TODO: remove this later
#define NDEBUG

#ifndef NDEBUG
#include <iostream>
#include <iomanip>
void printbuf(unsigned char *dataBuffer, std::size_t len) {
	int lineBytes(0);
	for (int j(0); j < len; ++j) {
		std::cout << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(dataBuffer[j]) << " ";
		++lineBytes;
		if (lineBytes == 33) {
			lineBytes = 0;
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
}
#endif

namespace Lyli {

class Camera::Impl {
public:
	Impl(Camera *camera_, const Usbpp::MassStorage::MSDevice &device_) :
		camera(camera_),
		device(device_)
	{
		// ensure that the device is opened
		device.open(true);
		
		// clear halt and reset the device, so it doesn't have to be reconnected manually in case of error
		/*device.clearHalt(LIBUSB_ENDPOINT_OUT | 0x02);
		device.clearHalt(LIBUSB_ENDPOINT_IN | 0x02);
		device.reset();*/
		/*if (! device.reset()) {
			std::cerr << "Cannot reset device!" << std::endl;
			return 1;
		}*/
		
		// claim the interface
		device.claimInterface(0);
	}
	
	~Impl() {
		device.close();
	}
	
	bool isReady() {
		Usbpp::MassStorage::CommandBlockWrapper cmdUnitReady(0, 0, 0, {0,0,0,0, 0,0,0,0, 0,0,0,0});
		
		std::unique_lock<std::mutex> cameraLock(cameraAccessMutex);
		bool ready = device.sendCommand(LIBUSB_ENDPOINT_OUT | 0x02, cmdUnitReady, nullptr).getStatus() ==
			Usbpp::MassStorage::CommandStatusWrapper::Status::PASSED;
		cameraLock.unlock();	
		
		return ready;
	}
	
	Usbpp::ByteBuffer downloadData() {
		Usbpp::ByteBuffer result;
		Usbpp::ByteBuffer response;
		
		// doesn't need locking, because the lock is already kept by caller
		
		// get the expected response length
		Usbpp::MassStorage::CommandBlockWrapper cmdGetLen(65536, 0x80, 0, {0xc6, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00});
		device.sendCommand(LIBUSB_ENDPOINT_OUT | 0x02, cmdGetLen, &response);
#ifndef NDEBUG
		std::size_t len(*reinterpret_cast<uint32_t*>(response.data()));
		std::cout << "expected length " << len << " bytes" << std::endl;
#endif
		
		// read the file list
		unsigned char packet(0);
		do {
			Usbpp::MassStorage::CommandBlockWrapper cmdGetData(65536, 0x80, 0, {0xc4, 00, 01, 00, 00, packet++, 00, 00, 00, 00, 00, 00});
			device.sendCommand(LIBUSB_ENDPOINT_OUT | 0x02, cmdGetData, &response);
			result.append(response);
		} while (response.size() == 65536);
		
		return result;
	}
	
	CameraInformation getCameraInformation() const {
		CameraInformation info;
		std::unique_lock<std::mutex> cameraLock(cameraAccessMutex);
		Usbpp::MassStorage::SCSI::InquiryResponse response(device.sendInquiry(0x02, 0));
		cameraLock.unlock();
		info.vendor = std::string((const char*) response.getVendorIdentification().data(), response.getVendorIdentification().size());
		info.product = std::string((const char*) response.getProductIdentification().data(), response.getProductIdentification().size());
		info.revision = std::string((const char*) response.getProductRevisionLevel().data(), response.getProductRevisionLevel().size());
		
		return info;
	}
	
	FileList getPictureList() {
		Usbpp::ByteBuffer response;
		
		std::unique_lock<std::mutex> cameraLock(cameraAccessMutex);
		
		// request the file list
		Usbpp::MassStorage::CommandBlockWrapper cmdReqFilelist(0, 0, 0, {0xc2, 00, 02, 00, 00, 00, 00, 00, 00, 00, 00, 00});
		device.sendCommand(LIBUSB_ENDPOINT_OUT | 0x02, cmdReqFilelist, &response);
		
		// get the data
		response = downloadData();
		
		cameraLock.unlock();
		
		return parseFileList(camera, response);
	}
	
	void getFile(std::ostream &out, const char *fileName)
	{
		Usbpp::ByteBuffer dataBuffer(65536);
		Usbpp::ByteBuffer response;
		
		std::size_t len(std::strlen(fileName) + 1);
		Usbpp::ByteBuffer fileBuf(reinterpret_cast<const uint8_t*>(fileName), len);
		
		std::unique_lock<std::mutex> cameraLock(cameraAccessMutex);
		
		// request the file
		Usbpp::MassStorage::CommandBlockWrapper cmdReqFile(len, 0, 0, {0xc2, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00});
		device.bulkTransferOut(LIBUSB_ENDPOINT_OUT | 0x02, cmdReqFile.getBuffer(), 0);
		device.bulkTransferOut(LIBUSB_ENDPOINT_OUT | 0x02, fileBuf, 0);
		
		// check the request status
		device.bulkTransferIn(LIBUSB_ENDPOINT_IN | 0x02, dataBuffer, 0);
		
		// get the data
		response = downloadData();
		
		cameraLock.unlock();
		
		out.write(reinterpret_cast<const char*>(response.data()), response.size());
		
#ifndef NDEBUG
		std::cout << "response length " << response.size() << std::endl;
		printbuf(response.data(), response.size());
#endif
	}
	
	Camera *camera;
	Usbpp::MassStorage::MSDevice device;
	
	mutable std::mutex cameraAccessMutex;
};

Camera::Camera() : pimpl(nullptr)
{

}

Camera::Camera(const Usbpp::MassStorage::MSDevice &device) : pimpl(new Impl(this, device))
{
	
}

Camera::~Camera()
{
	if (pimpl != nullptr) {
		delete pimpl;
		pimpl = nullptr;
	}
}

Camera::Camera(Camera&& other) noexcept
{
	pimpl = other.pimpl;
	pimpl->camera = this;
	other.pimpl = nullptr;
}

Camera& Camera::operator=(Camera&& other) noexcept
{
	if (this == &other) {
		return *this;
	}
	
	std::swap(pimpl, other.pimpl);
	pimpl->camera = this;
	other.pimpl->camera = &other;
	return *this;
}


void Camera::waitReady()
{
	while (! pimpl->isReady());
}

CameraInformation Camera::getCameraInformation() const
{
	assert(pimpl != nullptr);
	
	return pimpl->getCameraInformation();
}

void Camera::getFirmware(std::ostream &os)
{
	assert(pimpl != nullptr);
	
	pimpl->getFile(os, "A:\\FIRMWARE.TXT");
}

void Camera::getVCM(std::ostream &os)
{
	assert(pimpl != nullptr);
	
	pimpl->getFile(os, "A:\\VCM.TXT");
}

FileList Camera::getPictureList()
{
	assert(pimpl != nullptr);
	
	return pimpl->getPictureList();
}

void Camera::getFile(std::ostream &out, const std::string &fileName) const
{
	return pimpl->getFile(out, fileName.c_str());
}

CameraList getCameras(Usbpp::Context &context)
{
	CameraList cameras;
	
	std::vector<Usbpp::Device> devices(context.getDevices());
	
	for (Usbpp::Device dev : devices) {
		try {
			dev.open(true);
			libusb_device_descriptor descr(dev.getDescriptor());
			if (descr.idVendor == 0x24cf && descr.idProduct == 0x00a1) {
				cameras.push_back(Camera(dev));
			}
			else {
				dev.close();
			}
		}
		catch (const Usbpp::DeviceException &e) {
			// just silently ignore the exception and try the next device
			continue;
		}
	}
	
	return cameras;
}

}
