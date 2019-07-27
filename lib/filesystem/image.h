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

#ifndef LYLI_FILESYSTEM_IMAGE_H
#define LYLI_FILESYSTEM_IMAGE_H

#include <iosfwd>
#include <memory>
#include <string>

namespace Lyli {

class Camera;

namespace Filesystem {

class Image {
public:
	/**
	 * Default constructor.
	 */
	Image();
	/**
	 * Construct file for path.
	 * \param camera camera where the file is stored, it must be vaild for the whole lifetime
	 *               of the file object. Must not be null. File does NOT take ownership.
	 * \param path full path to file
	 * \param basename base name of the file (ie. the file name wihout leading path and without extension)
	 */
	Image(Camera *camera, const std::string &path, const std::string basename);
	/**
	 * A copy constructor
	 */
	Image(const Image& other);
	/**
	 * A move constructor
	 */
	Image(Image&& other) noexcept;
	/**
	 * A destructor.
	 */
	virtual ~Image();

	/**
	 * Assignment operator
	 */
	Image& operator=(const Image& other);
	/**
	 * Move assignment operator
	 */
	Image& operator=(Image&& other) noexcept;

	/**
	 * Get the file name.
	 * \return file name
	 */
	std::string getName();

	/**
	 * Store the image metadata to a given stream.
	 * \param os stream where the data are written to
	 */
	void getImageMetadata(std::ostream &os) const;
	/**
	 * Store the image data to a given stream.
	 * \param os stream where the data are written to
	 */
	void getImageData(std::ostream &os) const;

protected:
	/**
	 * Get the associated camera.
	 * \return the camera
	 */
	Camera *getCamera() const;
	/**
	 * Get the full path without the extension
	 * \return path
	 */
	std::string &getFullPath() const;

private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

}
}

#endif // LYLI_FILESYSTEM_IMAGE_H
