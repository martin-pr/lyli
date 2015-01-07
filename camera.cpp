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

#include "usbpp/buffer.h"
#include "usbpp/context.h"
#include "usbpp/msdevice.h"
#include "usbpp/cbw.h"
#include "usbpp/csw.h"
#include "usbpp/scsiinquiryresponse.h"

#include <cassert>
#include <cstring>
#include <ostream>

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
	Impl(const Usbpp::MassStorage::MSDevice &device_) : device(device_) {
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
		
		return device.sendCommand(LIBUSB_ENDPOINT_OUT | 0x02, cmdUnitReady, nullptr).getStatus() ==
			Usbpp::MassStorage::CommandStatusWrapper::Status::PASSED;
	}
	
	Usbpp::ByteBuffer downloadData() {
		Usbpp::ByteBuffer result;
		Usbpp::ByteBuffer response;
		
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
	
	CameraInformation getCameraInformation() {
		CameraInformation info;
		Usbpp::MassStorage::SCSI::InquiryResponse response(device.sendInquiry(0x02, 0));
		info.vendor = std::string((const char*) response.getVendorIdentification().data(), response.getVendorIdentification().size());
		info.product = std::string((const char*) response.getProductIdentification().data(), response.getProductIdentification().size());
		info.revision = std::string((const char*) response.getProductRevisionLevel().data(), response.getProductRevisionLevel().size());
		
		return info;
	}
	
	FileList getFileList() {
		Usbpp::ByteBuffer response;
		
		// request the file list
		Usbpp::MassStorage::CommandBlockWrapper cmdReqFilelist(0, 0, 0, {0xc2, 00, 02, 00, 00, 00, 00, 00, 00, 00, 00, 00});
		device.sendCommand(LIBUSB_ENDPOINT_OUT | 0x02, cmdReqFilelist, &response);
		
		// get the data
		response = downloadData();
		
		return parseFileList(response);
	}
	
	void getFile(std::ostream &out, const char *fileName) {
		int transferred;
		unsigned char dataBuffer[65536];
		Usbpp::ByteBuffer response;
		
		std::size_t len(std::strlen(fileName) + 1);
		
		// request the file
		Usbpp::MassStorage::CommandBlockWrapper cmdReqFile(len, 0, 0, {0xc2, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00});
		device.bulkTransfer(LIBUSB_ENDPOINT_OUT | 0x02, cmdReqFile.getData(), cmdReqFile.getDataLength(), &transferred, 0);
		device.bulkTransfer(LIBUSB_ENDPOINT_OUT | 0x02, (unsigned char*) fileName, len, &transferred, 0);
		
		// check the request status
		device.bulkTransfer(LIBUSB_ENDPOINT_IN | 0x02, dataBuffer, 65536, &transferred, 0);
		
		// get the data
		response = downloadData();
		
		out.write(reinterpret_cast<const char*>(response.data()), response.size());
		
#ifndef NDEBUG
		std::cout << "response length " << response.size() << std::endl;
		printbuf(response.data(), response.size());
#endif
	}
	
private:
	Usbpp::MassStorage::MSDevice device;
};

Camera::Camera() : pimpl(nullptr)
{

}

Camera::Camera(Camera::Impl* impl) : pimpl(impl)
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
	other.pimpl = nullptr;
}

Camera& Camera::operator=(Camera&& other) noexcept
{
	std::swap(pimpl, other.pimpl);
}


void Camera::waitReady()
{
	while (! pimpl->isReady());
}

CameraInformation Camera::getCameraInformation()
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

FileList Camera::getFileList()
{
	assert(pimpl != nullptr);
	
	return pimpl->getFileList();
}

void Camera::getImageMetadata(std::ostream& os, int id)
{
	assert(pimpl != nullptr);
	
	char fileName[256];
	std::snprintf(fileName, 256, "I:\\DCIM\\100PHOTO\\IMG_%04d.TXT", id);
	fileName[255] = '\0';
	return pimpl->getFile(os, fileName);
}

void Camera::getImageThumbnail(std::ostream& os, int id)
{
	assert(pimpl != nullptr);
	
	char fileName[256];
	std::snprintf(fileName, 256, "I:\\DCIM\\100PHOTO\\IMG_%04d.128", id);
	fileName[255] = '\0';
	return pimpl->getFile(os, fileName);
}

// TODO: add checksum control
// eg. http://piliopoulos.wordpress.com/2011/02/16/c-api-openssl-libcrypto-sample-code/
void Camera::getImageData(std::ostream& os, int id)
{
	assert(pimpl != nullptr);
	
	char fileName[256];
	std::snprintf(fileName, 256, "I:\\DCIM\\100PHOTO\\IMG_%04d.RAW", id);
	fileName[255] = '\0';
	return pimpl->getFile(os, fileName);
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
				cameras.push_back(Camera(new Camera::Impl(dev)));
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
