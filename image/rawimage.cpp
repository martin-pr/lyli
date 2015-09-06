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
namespace Image {

RawImage::RawImage(std::istream& is, std::size_t width, std::size_t height) :
m_data(height, width, CV_16UC3)
{
	unsigned char buf[3];
	std::uint16_t tmp;
	std::size_t pos(0);
	
	uint16_t *data = reinterpret_cast<uint16_t*>(m_data.data);
	for (std::size_t y = 0; y < height; ++y) {
		// this assumes that the x-dimension has even number of pixels
		// so two pixels can be read at once
		for (std::size_t x = 0; x < width;) {
			is.read(reinterpret_cast<char*>(buf), 3);
		
			// mask out the first twelve bits
			// the data are stored as big endian, so we have to fix that, too
			tmp = (buf[0] << 8) | (buf[1] & 0xF0);
			data[pos + getColorIndex(x, y)] = tmp;
			pos += 3;
			++x;
			
			// the second 12b
			tmp = ((buf[1] & 0xF) << 12) | (buf[2] << 4);
			data[pos + getColorIndex(x, y)] = tmp;
			pos += 3;
			++x;
		}
	}
	
	demosaic();
}

cv::Mat &RawImage::getData()
{
	return m_data;
}

// simple bilinear interpolation
void RawImage::demosaic()
{
	uint16_t *data = reinterpret_cast<uint16_t*>(m_data.data);
	const int Y_OFF = m_data.cols * 3; // the offset used when adding (or subtracting) 1 to y-dimension
	const int X_OFF = 3; // // the offset used when adding (or subtracting) 1 to x-dimension
	// bounds, the -1 is required as the computation use 1px neighborhood
	const int ROW_MAX = m_data.rows - 1;
	const int COL_MAX = m_data.cols - 1;
	// temp variables for keeping position
	int x, y, pos;
	// red lines and blue stripes
	for (y = 1; y < ROW_MAX; y += 2) {
		for (x = 2; x < COL_MAX; x += 2) {
			pos = (y * m_data.cols + x) * 3;
			// red line
			data[pos] = 0.5 * (data[pos - X_OFF]+ data[pos + X_OFF]);
			// blue stripe
			data[pos+2] = 0.5 * (data[pos - Y_OFF + 2] + data[pos + Y_OFF + 2]);
		}
	}
	// blue lines and red stripes
	for (y = 2; y < ROW_MAX; y += 2) {
		for (x = 1; x < COL_MAX; x += 2) {
			pos = (y * m_data.cols + x) * 3;
			// red stripe
			data[pos] = 0.5 * (data[pos - Y_OFF] + data[pos + Y_OFF]);
			// blue line
			data[pos + 2] = 0.5 * (data[pos - X_OFF + 2] + data[pos + X_OFF + 2]);
		}
	}
	// red middle, green #1
	for (y = 2; y < ROW_MAX; y += 2) {
		for (x = 2; x < COL_MAX; x += 2) {
			pos = (y * m_data.cols + x) * 3;
			data[pos] = bilinearInterpolation(data[pos - X_OFF + Y_OFF], data[pos - X_OFF - Y_OFF],
			                                  data[pos + X_OFF + Y_OFF], data[pos + X_OFF - Y_OFF]);
			data[pos + 1] = avgInterpolation(data[pos - X_OFF + 1], data[pos + X_OFF + 1],
			                                 data[pos - Y_OFF + 1], data[pos + Y_OFF + 1]);
		}
	}
	// blue middle, green #2
	for (y = 1; y < ROW_MAX; y += 2) {
		for (x = 1; x < COL_MAX; x += 2) {
			pos = (y * m_data.cols + x) * 3;
			data[pos + 2] = bilinearInterpolation(data[pos - X_OFF + Y_OFF + 2], data[pos - X_OFF - Y_OFF + 2],
			                                      data[pos + X_OFF + Y_OFF + 2], data[pos + X_OFF - Y_OFF + 2]);
			data[pos + 1] = avgInterpolation(data[pos - X_OFF + 1], data[pos + X_OFF + 1],
			                                 data[pos - Y_OFF + 1], data[pos + Y_OFF + 1]);
		}
	}

	// and now borders!
	// actually use a braindead solution - just extend the first valid row/col to the border
	// top
	m_data.row(1).copyTo(m_data.row(0));
	// bottom
	m_data.row(ROW_MAX - 1).copyTo(m_data.row(ROW_MAX));
	// left
	m_data.col(1).copyTo(m_data.col(0));
	// right
	m_data.col(COL_MAX - 1).copyTo(m_data.col(COL_MAX));
}

}
}
