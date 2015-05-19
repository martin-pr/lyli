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

#include "calibrator.h"

namespace Lyli {

class Calibrator::Impl {
public:
	cv::Mat image;
	cv::Mat calibrationImage;
};

Calibrator::Calibrator() : pimpl(new Impl) {

}

Calibrator::~Calibrator() {

}

void Calibrator::addImage(const cv::Mat& image) {
	pimpl->image = image;
}

void Calibrator::calibrate() {
	cv::Mat dst, greyMat, tmp, res;

	// convert to gray
	cv::cvtColor(pimpl->image, greyMat, CV_RGB2GRAY);
	greyMat.convertTo(greyMat, CV_8U, 1.0/256.0);

	// find edges and apply threshold
	cv::Laplacian(greyMat, dst, CV_8U, 3);
	cv::threshold(dst, tmp, 64, 255, cv::THRESH_BINARY);

	// filter out small specks
	float m[3][3] = { { 0.125, 0.125, 0.125 }, { 0.125, 0, 0.125 }, { 0.125, 0.125, 0.125 } };
	cv::Mat filterKernel(3, 3, CV_32F, m);
	cv::filter2D(tmp, dst, CV_8U , filterKernel, cv::Point(1, 1));
	cv::threshold(dst, dst, 95, 255, cv::THRESH_BINARY);

	// invert
	dst = dst.mul(tmp);
	dst = cv::Scalar::all(255) - dst;

	cv::Point anchor(1, 1);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), anchor);
	cv::erode(dst, tmp, element, anchor, 2);
	cv::dilate(tmp, dst, element, anchor, 1);

	// DEBUG: convert to the format expected for viewwing
	dst.convertTo(res, CV_16U, 256);
	cv::cvtColor(res, pimpl->calibrationImage, CV_GRAY2RGB);
}

cv::Mat &Calibrator::getcalibrationImage() const {
	return pimpl->calibrationImage;
}

}
