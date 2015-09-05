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

#include "preprocessor.h"

#include <cstdint>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace Lyli {
namespace Calibration {

/**
 * Compute optimal threshold for thresholding the edge image
 *
 * @return threshold
 */
std::uint8_t computeEdgeThreshold(const cv::Mat& edges) {
	// compute mean and use it as average
	return cv::mean(edges)[0];
}

void Preprocessor::preprocess(const cv::Mat& gray, cv::Mat& outMask) {
	cv::Mat tmp;

	// find edges and apply threshold
	cv::Laplacian(gray, outMask, CV_8U, 3);
	std::uint8_t threshold = computeEdgeThreshold(outMask);
	cv::threshold(outMask, tmp, threshold, Mask::OBJECT, cv::THRESH_BINARY);

	// filter out small specks
	float m[3][3] = { { 0.125, 0.125, 0.125 }, { 0.125, 0, 0.125 }, { 0.125, 0.125, 0.125 } };
	cv::Mat filterKernel(3, 3, CV_32F, m);
	cv::filter2D(tmp, outMask, CV_8U , filterKernel, cv::Point(1, 1));
	cv::threshold(outMask, outMask, 95, Mask::OBJECT, cv::THRESH_BINARY);

	// invert
	outMask = outMask.mul(tmp);
	outMask = cv::Scalar::all(Mask::OBJECT) - outMask;

	cv::Point anchor(1, 1);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), anchor);
	cv::erode(outMask, tmp, element, anchor, 2);
	cv::dilate(tmp, outMask, element, anchor, 1);
}

}
}
