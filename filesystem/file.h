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

#ifndef LYLI_FILE_H
#define LYLI_FILE_H

#include <iosfwd>
#include <memory>
#include <string>

namespace Lyli {

class Camera;

namespace Filesystem {

/**
 * An interface for downloadable files
 */
class File {
public:
	/**
	 * Default constructor.
	 */
	File();
	/**
	 * Construct file for path.
	 * \param camera camera where the file is stored, it must be vaild for the whole lifetime
	 *               of the file object. Must not be null. File does NOT take ownership.
	 * \param path full path to file
	 */
	File(Camera *camera, const std::string &path);
	/**
	 * A copy constructor
	 */
	File(const File& other);
	/**
	 * A move constructor
	 */
	File(File&& other) noexcept;
	/**
	 * A destructor.
	 */
	virtual ~File();

	/**
	 * Assignment operator
	 */
	File& operator=(const File& other);
	/**
	 * Move assignment operator
	 */
	File& operator=(File&& other) noexcept;

	/**
	 * Get the file name.
	 * \return file name
	 */
	virtual std::string getName();

	/**
	 * Download the file
	 * \param os output stream in which the file is written
	 */
	virtual void download(std::ostream &os);

private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

using FilePtr = std::shared_ptr<File>;

}
}

#endif // LYLI_FILE_H
