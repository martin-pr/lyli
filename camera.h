/*
 * This file is part of Litro, an application to control Lytro camera
 * Copyright (C) 2015  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
 *
 * Litro is free software: you can redistribute it and/or modify
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

#ifndef LITRO_DOWNLOADER_H_
#define LITRO_DOWNLOADER_H_

#include <cstdint>
#include <ctime>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace Usbpp {
class Context;
}

namespace Litro {

class Camera;
typedef std::vector<Camera> CameraList;

CameraList getCameras(Usbpp::Context &context);

struct FileListEntry {
	// the counter for file name
	int id;
	// a sha1 hash (stored as binary)
	uint8_t sha1[20];
	// a time the picture was taken
	std::time_t time;
};

typedef std::vector<FileListEntry> FileList;

struct CameraInformation {
	std::string vendor;
	std::string product;
	std::string revision;
};

class Camera {
private:
	class Impl;
	
public:
	Camera();
	~Camera();
	
	Camera(Camera &&other) noexcept;
	Camera &operator=(Camera &&other) noexcept;
	
	void waitReady();
	
	CameraInformation getCameraInformation();
	
	void getVersion();
	void getFirmware(std::ostream &os);
	void getVCM(std::ostream &os);
	
	FileList getFileList();
	
	// all image data
	// TODO: add progress
	void getImageMetadata(std::ostream &os, int id);
	void getImage128(std::ostream &os, int id);
	void getImageData(std::ostream &os, int id);
	
private:
	Impl *pimpl;
	
	friend CameraList getCameras(Usbpp::Context &context);
	Camera(Impl *impl);
	
	Camera(const Camera &other);
	Camera &operator=(const Camera &other);
};

}

#endif
