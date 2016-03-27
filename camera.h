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

#ifndef LYLI_CAMERA_H_
#define LYLI_CAMERA_H_

#include <memory>
#include <iosfwd>
#include <string>
#include <vector>

// forward declarations
namespace Usbpp {
class ByteBuffer;
class Context;
namespace MassStorage {
class MSDevice;
}
}

namespace Lyli {
class Context;
namespace Filesystem {
class FilesystemAccess;
}
}

namespace Lyli {

/**
 * Basic camera information
 */
struct CameraInformation {
	std::string vendor;
	std::string product;
	std::string revision;
};

/**
 * Interface to a single camera
 */
class Camera {
public:
	/**
	 * A destructor (required for pimpl std::unique_ptr).
	 */
	~Camera();

	/**
	 * A move constructor.
	 */
	Camera(Camera &&other) noexcept;
	/**
	 * A move assignment operator.
	 */
	Camera &operator=(Camera &&other) noexcept;

	/**
	 * Wait for the camera to become ready to communicate.
	 *
	 * The fcuntions blocks until the camera becomes ready.
	 */
	void waitReady();

	/**
	 * Get basic camera inforamtion
	 * \return the camera information
	 */
	CameraInformation getCameraInformation() const;

	/**
	 * Get specific file from camera
	 * \param out output stream where the file is written to.
	 * \param fileName the path to file to download
	 */
	void getFile(std::ostream &out, const std::string &fileName) const;

	/**
	 * Get an object abstracting access to files on camera
	 * \return objects providing file access
	 */
	Filesystem::FilesystemAccess getFilesystemAccess();

private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

	friend class ::Lyli::Context;
	friend class ::Lyli::Filesystem::FilesystemAccess;;

	Camera(const Usbpp::MassStorage::MSDevice &device);

	Usbpp::ByteBuffer getPictureList();

	Camera(const Camera &other);
	Camera &operator=(const Camera &other);
};

}

#endif
