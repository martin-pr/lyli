/*
 * This file is part of Lyli-Qt, a GUI to control Lytro camera
 * Copyright (C) 2015  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "lytroimage.h"

#include <QtGui/QImage>

#include <cstddef>
#include <cstdint>
#include <fstream>

#include <rawimage.h>

namespace {

constexpr std::size_t IMG_WIDTH=3280;
constexpr std::size_t IMG_HEIGHT=3280;

}

LytroImage::LytroImage(const char *file)
{
	m_image = new QImage(IMG_WIDTH, IMG_HEIGHT, QImage::Format_RGB32);
	std::fstream fin(file, std::fstream::in | std::fstream::binary);
	
	Lyli::RawImage image(fin, IMG_WIDTH, IMG_HEIGHT);
	fin.close();
	
	uint16_t *rawImage = image.getData();
	// combine the images
	std::size_t pos(0);
	for(std::size_t y = 0; y < IMG_HEIGHT; ++y) {
		for(std::size_t x = 0; x < IMG_WIDTH; ++x) {
			m_image->setPixel(x, y, qRgb(
				rawImage[pos],
				rawImage[pos + 1],
				rawImage[pos + 2]));
			pos += 3;
		}
	}
}

LytroImage::~LytroImage()
{
	delete m_image;
}

const QImage *LytroImage::getQImage() const
{
	return m_image;
}
