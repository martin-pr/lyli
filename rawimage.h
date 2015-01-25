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

#ifndef LYLI_RAWIMAGE_H_
#define LYLI_RAWIMAGE_H_

#include <cstddef>
#include <cstdint>
#include <iostream>

namespace Lyli {

/** A class providing a simple interface for accessing the Lytro RAW images.
 * 
 */
class RawImage {
public:
	/** Construct the image.
	 * 
	 * \param is input stream to the opened .RAW file
	 */
	RawImage(std::istream &is, std::size_t width, std::size_t height);
	
	/** Get processed image data
	 * 
	 * \return pointer to a buffer containing width*height RGB uint16_t pixels
	 */
	uint16_t* getData();
	
private:
	uint16_t* m_data;
	std::size_t m_width;
	std::size_t m_height;
	
	void demosaic();
};

}

#endif
