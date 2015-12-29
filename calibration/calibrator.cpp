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
#include "fftpreprocessor.h"
#include "lensdetector.h"
//#include "lensfilter.h"
#include "pointgrid.h"

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

#include <opencv2/calib3d/calib3d.hpp>
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
	class ZoomStepCompare {
	public:
		bool operator()(const Lyli::Image::Metadata::Devices::Lens& a, const Lyli::Image::Metadata::Devices::Lens& b) const {
			return a.getZoomstep() < b.getZoomstep();
		}
	};

	using LineGridMap = std::map<Lyli::Image::Metadata::Devices::Lens, PointGrid, ZoomStepCompare>;
	LineGridMap linegridmap;

	/**
	 * Create target line grid.
	 *
	 * The target line grid is an "optimal" line grid. The calibration
	 * tries to find mapping from the image line grid to the target line grid.
	 */
	PointGrid createTarget(const PointGrid &grid);
};

/*PointGrid Calibrator::Impl::createTarget(const PointGrid &grid) {
	// createTarget makes use of the fact that the points are shared
	// between both vertical and horintal lines
	PointGrid result(grid);

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
}*/

Calibrator::Calibrator() : pimpl(new Impl) {

}

Calibrator::~Calibrator() {

}

void Calibrator::processImage(const Lyli::Image::RawImage &image, const Lyli::Image::Metadata &metadata) {
	cv::Mat dst, greyMat;

	// convert to gray
	cv::cvtColor(image.getData(), greyMat, CV_RGB2GRAY);
	greyMat.convertTo(greyMat, CV_8U, 1.0/256.0);

	// preprocess the image - create a mask for the microlens array
	FFTPreprocessor preprocessor;
	preprocessor.preprocess(greyMat, dst);

	// construct line grid
	LensDetector lensDetector;
	PointGrid pointGrid = lensDetector.detect(greyMat, dst);

	// store the line grid with the metadata
	pimpl->linegridmap.emplace(metadata.getDevices().getLens(), pointGrid);
}

CalibrationData Calibrator::calibrate() {
	// first create a destination line grid that is computed as an average of all line grids

	return CalibrationData();
	/*cv::Mat dst, greyMat, tmp;

	// convert to gray
	cv::cvtColor(pimpl->image, greyMat, CV_RGB2GRAY);
	greyMat.convertTo(greyMat, CV_8U, 1.0/256.0);

	// preprocess the image - create a mask for the microlens array
	FFTPreprocessor preprocessor;
	preprocessor.preprocess(greyMat, dst);

	LensDetector lensDetector;
	LineGrid pointGrid = lensDetector.detect(greyMat, dst);
	LineGrid targetGrid = pimpl->createTarget(pointGrid);

	//LensFilter lensFilter;
	//lineMap = lensFilter.filter(lineMap);

	// use the opencv lens calibration
	// create object points (source, in 3D)
	std::vector<std::vector<cv::Point3f> > objectPoints(1);
	for (const auto &line : pointGrid.getHorizontalLines()) {
		for (const auto &point : line) {
			objectPoints[0].push_back(cv::Point3f(point->x, point->y, 0.0));
		}
	}
	// create image points (destination, in 2D)
	std::vector<std::vector<cv::Point2f> > imagePoints(1);
	for (const auto &line : targetGrid.getHorizontalLines()) {
		for (const auto &point : line) {
			imagePoints[0].push_back(cv::Point2f(point->x, point->y));
		}
	}
	// remaining parameters
	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
	cv::Mat distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
	std::vector<cv::Mat> rvecs, tvecs;
	std::vector<float> reprojErrs;

	// run the calibration
	cv::calibrateCamera(objectPoints, imagePoints, pimpl->image.size(), cameraMatrix, distCoeffs, rvecs, tvecs);

	// get the 2D affine transform
	cv::Mat rotation3D;
	cv::Rodrigues(rvecs[0], rotation3D); // get 3D rotation
	cv::Mat rotation = cv::Mat::eye(3, 3, CV_64F);
	cv::Mat editRot = rotation(cv::Range(0, 2), cv::Range(0, 2));
	rotation3D(cv::Range(0, 2), cv::Range(0, 2)).copyTo(editRot); // we we need 2D, so we set z = 0 => top left submatrix suffice

	cv::Mat translation = cv::Mat::eye(3, 3, CV_64F);
	cv::Mat editTransl = translation.col(2).rowRange(0, 2);
	tvecs[0].rowRange(0, 2).copyTo(editTransl);

	std::cout << "camera matrix" << std::endl << cameraMatrix << std::endl;
	std::cout << "distortion coefficients" << std::endl << distCoeffs << std::endl; // radial and tangential distortion
	std::cout << "rotation" << std::endl << rotation << std::endl;
	std::cout << "translation" << std::endl << translation << std::endl;

	pimpl->m_calibrationData = CalibrationData(cameraMatrix, distCoeffs, translation, rotation);

	// DEBUG: draw lines
	dst = cv::Scalar(256, 256, 256);
	const PtrLineList &linesHorizontal = pointGrid.getHorizontalLines();
	bool alternator = false;
	for (const auto &line : linesHorizontal) {
		cv::Scalar color = alternator ? cv::Scalar(0, 0, 0) : cv::Scalar(128, 128, 128);
		for (std::size_t i = 1; i < line.size(); ++i) {
			cv::line(dst, *(line.at(i-1)), *(line.at(i)), color);
		}
		alternator = !alternator;
	}
	const PtrLineList &linesVerticalEven = pointGrid.getVerticalLinesEven();
	for (const auto &line : linesVerticalEven) {
		for (std::size_t i = 3; i < line.size(); i+=2) {
			cv::line(dst, *(line.at(i-2)), *(line.at(i)), cv::Scalar(128, 128, 128));
		}
	}
	const PtrLineList &linesVerticalOdd = pointGrid.getVerticalLinesOdd();
	for (const auto &line : linesVerticalOdd) {
		for (std::size_t i = 2; i < line.size(); i+=2) {
			cv::line(dst, *(line.at(i-2)), *(line.at(i)), cv::Scalar(0, 0, 0));
		}
	}

	// DEBUG: undistort the grid
	cv::Mat transform = translation.inv() * rotation * translation;
	cv::undistort(dst, tmp, cameraMatrix, distCoeffs);
	cv::warpAffine(tmp, dst, transform.rowRange(0,2), dst.size());

	// DEBUG: convert to the format expected for viewing
	dst = dst.t();
	dst.convertTo(tmp, CV_16U, 255);
	cv::cvtColor(tmp, pimpl->calibrationImage, CV_GRAY2RGB);*/
}

}
}
