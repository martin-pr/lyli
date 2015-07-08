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

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>

#include <calibration/calibrator.h>
#include <rawimage.h>

namespace {

constexpr std::size_t IMG_WIDTH=3280;
constexpr std::size_t IMG_HEIGHT=3280;

}

unsigned char LytroImage::m_gamma[4096];

LytroImage::LytroImage() : m_image(nullptr)
{

}

LytroImage::LytroImage(const char *file)
{
	m_image = new QImage(IMG_WIDTH, IMG_HEIGHT, QImage::Format_RGB32);
	std::fstream fin(file, std::fstream::in | std::fstream::binary);
	
	Lyli::RawImage image(fin, IMG_WIDTH, IMG_HEIGHT);
	fin.close();
	
	// DEBUG: show prerocessed calibration image
	/*Lyli::Calibrator calibrator;
	calibrator.addImage(image.getData());
	calibrator.calibrate();
	uint16_t *rawImage = reinterpret_cast<uint16_t*>(calibrator.getcalibrationImage().data);*/

	uint16_t *rawImage = reinterpret_cast<uint16_t*>(image.getData().data);
	// combine the images
	std::size_t pos(0);
	for(std::size_t y = 0; y < IMG_HEIGHT; ++y) {
		for(std::size_t x = 0; x < IMG_WIDTH; ++x) {
			m_image->setPixel(x, y, qRgb(
				m_gamma[rawImage[pos] >> 4],
				m_gamma[rawImage[pos+1] >> 4],
				m_gamma[rawImage[pos+2] >> 4]));
			pos += 3;
		}
	}
}

LytroImage::~LytroImage()
{
	if (m_image != nullptr) {
		delete m_image;
	}
}

LytroImage::LytroImage(const LytroImage& other)
{
	m_image = new QImage(*other.m_image);
}

LytroImage::LytroImage(LytroImage&& other)
{
	m_image = other.m_image;
	other.m_image = nullptr;
}

LytroImage& LytroImage::operator=(const LytroImage& other)
{
	if (this == &other) {
		return *this;
	}
	LytroImage tmp(other);
	std::swap(m_image, tmp.m_image);
	return *this;
}

LytroImage& LytroImage::operator=(LytroImage&& other)
{
	if (this == &other) {
		return *this;
	}
	m_image = other.m_image;
	other.m_image = nullptr;
	return *this;
}

void LytroImage::init()
{
	static double gamma = 1.0/2.2;
	for (uint16_t i = 0; i < 4096; ++i) {
		double tmp = i / 4096.0;
		m_gamma[i] = std::pow(tmp, gamma) * 255;
	}
}

const QImage *LytroImage::getQImage() const
{
	return m_image;
}
