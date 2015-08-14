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

	/**
	 * Create target line grid.
	 *
	 * The target line grid is an "optimal" line grid. The calibration
	 * tries to find mapping from the image line grid to the target line grid.
	 */
	LineGrid createTarget(const LineGrid &grid);
};

LineGrid Calibrator::Impl::createTarget(const LineGrid &grid) {
	// createTarget makes use of the fact that the points are shared
	// between both vertical and horintal lines
	LineGrid result(grid);

	// set the x value in each horizontal line to the average x
	for (auto &line : result.getHorizontalLines()) {
		// first compute average
		double sum = 0.0;
		double count = 0.0;
		for (std::size_t i = line.size() / 3; i < 2 * line.size() / 3; ++i) {
			sum += line[i]->x;
			count += 1.0;
		}
		double avg = sum / count;
		// set the average to all points
		for (auto &point : line) {
			point->x = avg;
		}
	}
	// set the y value in each vertical line for odd rows to the average
	for (auto &line : result.getVerticalLinesOdd()) {
		// first compute average
		double sum = 0.0;
		double count = 0.0;
		std::size_t start = line.size() / 3;
		start = (start & 1) == 0 ? start : start - 1;
		for (std::size_t i = start; i < 2 * line.size() / 3; i += 2) {
			sum += line[i]->y;
			count += 1.0;
		}
		double avg = sum / count;
		// set the average to all points
		for (std::size_t i = 0; i < line.size(); i += 2) {
			line[i]->y = avg;
		}
	}
	// set the y value in each vertical line for even rows to the average
	for (auto &line : result.getVerticalLinesEven()) {
		// first compute average
		double sum = 0.0;
		double count = 0.0;
		std::size_t start = line.size() / 3;
		start = (start & 1) != 0 ? start : start - 1;
		for (std::size_t i = start; i < 2 * line.size() / 3; i += 2) {
			sum += line[i]->y;
			count += 1.0;
		}
		double avg = sum / count;
		// set the average to all points
		for (std::size_t i = 1; i < line.size(); i += 2) {
			line[i]->y = avg;
		}
	}

	return result;
}

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
	LineGrid targetGrid = pimpl->createTarget(lineGrid);

	/*LensFilter lensFilter;
	lineMap = lensFilter.filter(lineMap);*/

	// DEBUG: draw lines
	dst = cv::Scalar(256, 256, 256);
	const PtrLineList &linesHorizontal = lineGrid.getHorizontalLines();
	for (const auto &line : linesHorizontal) {
		for (std::size_t i = 1; i < line.size(); ++i) {
			cv::line(dst, *(line.at(i-1)), *(line.at(i)), cv::Scalar(0, 0, 0));
		}
	}
	const PtrLineList &linesVertical = lineGrid.getVerticalLinesEven();
	for (const auto &line : linesVertical) {
		for (std::size_t i = 1; i < line.size(); ++i) {
			cv::line(dst, *(line.at(i-1)), *(line.at(i)), cv::Scalar(0, 0, 0));
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
