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

#ifndef LYLI_DOWNLOADER_H_
#define LYLI_DOWNLOADER_H_

#include <cstdint>
#include <ctime>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "filelist.h"

namespace Usbpp {
class Context;
namespace MassStorage {
class MSDevice;
}
}

namespace Lyli {

class Camera;
typedef std::vector<Camera> CameraList;

CameraList getCameras(Usbpp::Context &context);

struct CameraInformation {
	std::string vendor;
	std::string product;
	std::string revision;
};

class Camera {
public:
	Camera();
	~Camera();
	
	Camera(Camera &&other) noexcept;
	Camera &operator=(Camera &&other) noexcept;
	
	void waitReady();
	
	CameraInformation getCameraInformation() const;
	
	void getVersion();
	void getFirmware(std::ostream &os);
	void getVCM(std::ostream &os);
	
	FileList getPictureList();
	void getFile(std::ostream &out, const std::string &fileName) const;
	
private:
	class Impl;
	Impl *pimpl;
	
	friend CameraList getCameras(Usbpp::Context &context);
	Camera(const Usbpp::MassStorage::MSDevice &device);
	
	Camera(const Camera &other);
	Camera &operator=(const Camera &other);
};

}

#endif
