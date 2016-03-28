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

#include <calibration/calibrationdata.h>
#include <calibration/calibrator.h>
#include <image/lightfieldimage.h>
#include <image/metadata.h>
#include <image/rawimage.h>

#include <config/lyliconfig.h>

namespace {

constexpr std::size_t IMG_WIDTH=3280;
constexpr std::size_t IMG_HEIGHT=3280;

}

unsigned char LytroImage::m_gamma[4096];

LytroImage::LytroImage() : m_image(nullptr) {

}

LytroImage::LytroImage(const char *file) {
	std::fstream fin(file, std::fstream::in | std::fstream::binary);

	Lyli::Image::RawImage rawimg(fin, IMG_WIDTH, IMG_HEIGHT);
	fin.close();

	std::string metafile(file);
	metafile = metafile.substr(0, metafile.find_last_of(".")) + ".TXT";
	std::fstream finmeta(metafile, std::fstream::in | std::fstream::binary);
	Lyli::Image::Metadata metadata(finmeta);

	cv::Mat showImg;
	std::string serial(metadata.getPrivatemetadata().getCamera().getSerialnumber());
	std::unique_ptr<::Lyli::Calibration::CalibrationData> calibration = LyliConfig::readCalibrationData(serial);
	if (calibration) {
		// simple color img
		Lyli::Image::LightfieldImage lightfieldimg(rawimg, metadata, *calibration);
		showImg = lightfieldimg.getData();
	}
	else {
		// fallback
		showImg = rawimg.getData();
	}

	// show the image
	uint16_t *rawImage = reinterpret_cast<uint16_t*>(showImg.data);
	m_image = new QImage(showImg.cols, showImg.rows, QImage::Format_RGB32);
	std::size_t pos(0);
	for (std::size_t y = 0; y < static_cast<std::size_t>(showImg.rows); ++y) {
		for (std::size_t x = 0; x < static_cast<std::size_t>(showImg.cols); ++x) {
			m_image->setPixel(x, y, qRgb(
								m_gamma[rawImage[pos] >> 4],
								m_gamma[rawImage[pos+1] >> 4],
								m_gamma[rawImage[pos+2] >> 4]));
			pos += 3;
		}
	}
}

LytroImage::~LytroImage() {
	if (m_image != nullptr) {
		delete m_image;
	}
}

LytroImage::LytroImage(const LytroImage& other) {
	m_image = new QImage(*other.m_image);
}

LytroImage::LytroImage(LytroImage&& other) : m_image(other.m_image) {
	other.m_image = nullptr;
}

LytroImage& LytroImage::operator=(const LytroImage& other) {
	if (this != &other) {
		LytroImage tmp(other);
		std::swap(m_image, tmp.m_image);
	}
	return *this;
}

LytroImage& LytroImage::operator=(LytroImage&& other) {
	if (this != &other) {
		if (m_image) {
			delete m_image;
		}
		m_image = other.m_image;
		other.m_image = nullptr;
	}
	return *this;
}

void LytroImage::init() {
	static double gamma = 1.0/2.2;
	for (uint16_t i = 0; i < 4096; ++i) {
		double tmp = i / 4096.0;
		m_gamma[i] = std::pow(tmp, gamma) * 255;
	}
}

const QImage *LytroImage::getQImage() const {
	return m_image;
}
