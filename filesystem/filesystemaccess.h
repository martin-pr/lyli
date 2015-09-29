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

#ifndef LYLI_FILESYSTEM_FILESYSTEMACCESS_H
#define LYLI_FILESYSTEM_FILESYSTEMACCESS_H

#include <iosfwd>
#include <memory>

#include "filelist.h"

namespace Lyli {

class Camera;

namespace Filesystem {

class FilesystemAccess {
public:
	/**
	 * Construct a new filesystem access to a camera
	 * \param camera the accessed camera
	 */
	FilesystemAccess(Camera *camera);

	/**
	 * A copy constructor.
	 */
	FilesystemAccess(const FilesystemAccess &other);

	/**
	 *  A destructor.
	 */
	~FilesystemAccess();

	/**
	 * Store FIRMWARE.TXT to a stream.
	 * \param os stream where the data are written to
	 */
	void getFirmware(std::ostream &os);
	/**
	 * Store VCM.TXT (a file with some basic camera information) to a stream.
	 * \param os stream where the data are written to
	 */
	void getVCM(std::ostream &os);
	/**
	 * Get list of photos.
	 * \return list of photos
	 */
	PhotoList getPictureList();
	/**
	 * Get list of calibration images
	 * \return list of calibration images
	 */
	ImageList getCalibrationList();
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

}
}

#endif // LYLI_FILESYSTEM_FILESYSTEMACCESS_H
