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
#include "calibrationdata.h"

#include <utility>

#include <opencv2/core/core.hpp>

namespace Lyli {
namespace Calibration {

class CalibrationData::Impl {
public:
	Impl(const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs, const cv::Mat& translation, double rotation) {
		cameraMatrix.copyTo(m_cameraMatrix);
		distCoeffs.copyTo(m_distCoeffs);
		translation.copyTo(m_translation);
		m_rotation = rotation;
	}

	cv::Mat m_cameraMatrix;
	cv::Mat m_distCoeffs;
	cv::Mat m_translation;
	double m_rotation;
};

CalibrationData::CalibrationData() {

}

CalibrationData::CalibrationData(const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs, const cv::Mat& translation, double rotation) :
	pimpl(new Impl(cameraMatrix, distCoeffs, translation, rotation)) {

}

CalibrationData::~CalibrationData() {

}

CalibrationData::CalibrationData(const CalibrationData &other) :
	pimpl(new Impl(other.pimpl->m_cameraMatrix, other.pimpl->m_distCoeffs, other.pimpl->m_translation, other.pimpl->m_rotation)) {

}

CalibrationData& CalibrationData::operator=(const CalibrationData &other) {
	if(this != &other) {
		CalibrationData tmp(other.pimpl->m_cameraMatrix, other.pimpl->m_distCoeffs, other.pimpl->m_translation, other.pimpl->m_rotation);
		std::swap(pimpl, tmp.pimpl);
	}

	return *this;
}

CalibrationData::CalibrationData(CalibrationData &&other) noexcept : pimpl(std::move(other.pimpl)) {

}

CalibrationData& CalibrationData::operator=(CalibrationData &&other) noexcept {
	if (this != &other) {
		pimpl = std::move(other.pimpl);
	}

	return *this;
}

cv::Mat& CalibrationData::getCameraMatrix() const {
	return pimpl->m_cameraMatrix;
}

cv::Mat& CalibrationData::getDistCoeffs() const {
	return pimpl->m_distCoeffs;
}

cv::Mat& CalibrationData::getTranslation() const {
	return pimpl->m_translation;
}

double CalibrationData::getRotation() const {
	return pimpl->m_rotation;
}

}
}
