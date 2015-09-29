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

#ifndef LYLI_FILESYSTEM_PHOTO_H
#define LYLI_FILESYSTEM_PHOTO_H

#include "image.h"

#include <ctime>
#include <memory>

namespace Lyli {
namespace Filesystem {

using  Sha1Array = std::array<uint8_t, 20>;

class Photo : public Image {
public:
	Photo();
	Photo(Camera *camera, const std::string &dirBase, int dirId, const std::string &fileBase, int fileId, const Sha1Array &sha1, const std::time_t &time);
	Photo(const Photo& other);
	virtual ~Photo();
	Photo& operator=(const Photo& other);

	/**
	 * Time the picture was taken.
	 */
	std::time_t getTime() const;

	/**
	 * Store the image thumbnail to a given stream.
	 * \param os stream where the data are written to
	 */
	void getImageThumbnail(std::ostream &os) const;
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

}
}

#endif // LYLI_FILESYSTEM_PHOTO_H
