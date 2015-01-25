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

#include "rawimage.h"

namespace {

/** Return the index of a color corresponding to the value in bayer filter.
 * 
 * The function takes an x,y position in the image as the input and returns
 * the color that is stored at that position.
 * 
 * @return 0 - red, 1 - green, 2 - blue
 */
std::size_t getColorIndex(std::size_t x, std::size_t y) {
	if ((y & 1) != 0) {
		// even lines - red/green
		if ((x & 1) != 0) {
			return 0;
		}
		else {
			return 1;
		}
	}
	else {
		// odd lines - blue/green
		if ((x & 1) != 0) {
			return 1;
		}
		else {
			return 2;
		}
	}
}

uint16_t bilinearInterpolation(double f00, double f01, double f10, double f11) {
	double a = 0.5 * (f00 + f10);
	double b = 0.5 * (f01 + f11);
	return 0.5 * (a + b);
}

uint16_t avgInterpolation(double f00, double f01, double f10, double f11) {
	return 0.25 * (f00 + f01 + f10 + f11);
}

}

namespace Lyli {

RawImage::RawImage(std::istream& is, std::size_t width, std::size_t height) :
m_data(new uint16_t[width*height*3]()), m_width(width), m_height(height)
{
	char buf[3];
	std::uint16_t tmp;
	std::size_t pos(0);
	
	for (int y = 0; y < m_height; ++y) {
		// this assumes that the x-dimension has even number of pixels
		// so two pixels can be read at once
		for (int x = 0; x < m_width;) {
			is.read(buf, 3);
		
			// mask out the first twelve bits
			// the data are stored as big endian, so we have to fix that, too
			tmp = ((buf[1] & 0xF0) << 8) | buf[0];
			m_data[pos + getColorIndex(x, y)] = tmp;
			pos += 3;
			++x;
			
			// the second 12b
			tmp = ((buf[2] & 0xF) << 12) | ((buf[1] & 0xF) << 4) | ((buf[2] >> 4) & 0xF);
			m_data[pos + getColorIndex(x, y)] = tmp;
			pos += 3;
			++x;
		}
	}
	
	demosaic();
}

uint16_t* RawImage::getData()
{
	return m_data;
}

// simple bilinear interpolation
void RawImage::demosaic()
{
	const std::size_t Y_OFF = m_width * 3; // the offset used when adding (or subtracting) 1 to y-dimension
	const std::size_t X_OFF = 3; // // the offset used when adding (or subtracting) 1 to x-dimension
	std::size_t x, y, pos;
	// red lines and blue stripes
	for (y = 1; y < m_height; y += 2) {
		for (x = 2; x < m_width; x += 2) {
			pos = (y * m_width + x) * 3;
			// red line
			m_data[pos] = 0.5 * (m_data[pos - X_OFF]+ m_data[pos + X_OFF]);
			// blue stripe
			m_data[pos+2] = 0.5 * (m_data[pos - Y_OFF + 2] + m_data[pos + Y_OFF + 2]);
		}
	}
	// blue lines and red stripes
	for (y = 2; y < m_height; y += 2) {
		for (x = 1; x < m_width; x += 2) {
			pos = (y * m_width + x) * 3;
			// red stripe
			m_data[pos] = 0.5 * (m_data[pos - Y_OFF] + m_data[pos + Y_OFF]);
			// blue line
			m_data[pos + 2] = 0.5 * (m_data[pos - X_OFF + 2] + m_data[pos + X_OFF + 2]);
		}
	}
	// red middle, green #1
	for (y = 2; y < m_height; y += 2) {
		for (x = 2; x < m_width; x += 2) {
			pos = (y * m_width + x) * 3;
			m_data[pos] = bilinearInterpolation(m_data[pos - X_OFF + Y_OFF], m_data[pos - X_OFF - Y_OFF],
			                                    m_data[pos + X_OFF + Y_OFF], m_data[pos + X_OFF - Y_OFF]);
			m_data[pos + 1] = avgInterpolation(m_data[pos - X_OFF + 1], m_data[pos + X_OFF + 1],
			                                   m_data[pos - Y_OFF + 1], m_data[pos + Y_OFF + 1]);
		}
	}
	// blue middle, green #2
	for (y = 1; y < m_height; y += 2) {
		for (x = 1; x < m_width; x += 2) {
			pos = (y * m_width + x) * 3;
			m_data[pos + 2] = bilinearInterpolation(m_data[pos - X_OFF + Y_OFF + 2], m_data[pos - X_OFF - Y_OFF + 2],
			                                        m_data[pos + X_OFF + Y_OFF + 2], m_data[pos + X_OFF - Y_OFF + 2]);
			m_data[pos + 1] = avgInterpolation(m_data[pos - X_OFF + 1], m_data[pos + X_OFF + 1],
			                                   m_data[pos - Y_OFF + 1], m_data[pos + Y_OFF + 1]);
		}
	}
}


}