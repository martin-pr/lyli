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
#include "preprocessor.h"
#include "lensdetector.h"
//#include "lensfilter.h"
#include "linegrid.h"
#include "linemap.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace Lyli {
namespace Calibration {

PreprocessorInterface::PreprocessorInterface() {

}

PreprocessorInterface::~PreprocessorInterface() {

}

LensDetectorInterface::LensDetectorInterface() {

}

LensDetectorInterface::~LensDetectorInterface() {

}

LensFilterInterface::LensFilterInterface() {

}

LensFilterInterface::~LensFilterInterface() {

}

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
	cv::Mat dst, greyMat, tmp;

	// convert to gray
	cv::cvtColor(pimpl->image, greyMat, CV_RGB2GRAY);
	greyMat.convertTo(greyMat, CV_8U, 1.0/256.0);

	// preprocess the image - create a mask for the microlens array
	Preprocessor preprocessor;
	preprocessor.preprocess(greyMat, dst);

	LensDetector lensDetector;
	LineGrid lineGrid = lensDetector.detect(greyMat, dst);

	/*LensFilter lensFilter;
	lineMap = lensFilter.filter(lineMap);*/

	// DEBUG: draw lines
	LineMap lineMap = lineGrid.getHorizontalMap();
	dst = cv::Scalar(256, 256, 256);
	for (auto &line : lineMap) {
		for (std::size_t i = 1; i < line->size(); ++i) {
			cv::line(dst, *(line->at(i-1)), *(line->at(i)), cv::Scalar(0, 0, 0));
		}
	}
	dst = dst.t();

	// DEBUG: convert to the format expected for viewwing
	dst.convertTo(tmp, CV_16U, 256);
	cv::cvtColor(tmp, pimpl->calibrationImage, CV_GRAY2RGB);
}

cv::Mat &Calibrator::getcalibrationImage() const {
	return pimpl->calibrationImage;
}

}
}
