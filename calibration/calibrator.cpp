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
#include "gridmapper.h"
#include "gridmath.h"
#include "linegrid.h"
#include "pointgrid.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <tbb/combinable.h>
#include <tbb/parallel_for_each.h>
#include <tbb/spin_mutex.h>

#include <image/metadata.h>

// BEGIN DEBUG
#include <sstream>
#include <opencv/highgui.h>
// END DEBUG

namespace {

/**
 * Image size in pixels
 */
static constexpr auto IMAGE_SIZE = 3280;
/**
 * Sensor size in metres
 * according to specification at:
 * https://store.lytro.com/collections/the-first-generation-product-list/products/first-generation-lytro-camera-8gb
 * the sensor is 4.6 x 4.6 mm
 */
static constexpr double SENSOR_SIZE = 0.0046;

void drawLineGrid(const char* file, const Lyli::Calibration::LineGrid &lineGrid) {


	cv::Mat dst(IMAGE_SIZE, IMAGE_SIZE, CV_8UC3);
	dst = cv::Scalar(256, 256, 256);
	const auto &linesHorizontal = lineGrid.getHorizontalLines();
	for (const auto &line : linesHorizontal) {
		cv::Scalar color = line.subgrid == Lyli::Calibration::SubGrid::SUBGRID_A ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 255, 0);
		cv::line(dst, cv::Point2f(0, line.position), cv::Point2f(IMAGE_SIZE, line.position), color);
	}
	const auto &linesVertical = lineGrid.getVerticalLines();
	for (const auto &line : linesVertical) {
		cv::Scalar color = line.subgrid == Lyli::Calibration::SubGrid::SUBGRID_A ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 255, 0);
		cv::line(dst, cv::Point2f(line.position, 0), cv::Point2f(line.position, IMAGE_SIZE), color);
	}

	cv::imwrite(file, dst);
}

class ZoomFocusHash {
public:
	std::size_t operator()(const Lyli::Image::Metadata::Devices::Lens &lens) const {
		const std::size_t zoomhash = std::hash<int>()(lens.getZoomstep());
		const std::size_t focushash = std::hash<int>()(lens.getFocusstep());
		return (zoomhash<<1) + zoomhash + focushash;
	}
};

class ZoomFocusComparator {
public:
	bool operator()(const Lyli::Image::Metadata::Devices::Lens &a, const Lyli::Image::Metadata::Devices::Lens &b) const {
		return (a.getZoomstep() == b.getZoomstep()) && (a.getFocusstep() == b.getFocusstep());
	}
};

using Cluster = std::vector<std::size_t>;
using ClusterMap = std::unordered_map<Lyli::Image::Metadata::Devices::Lens, Cluster, ZoomFocusHash, ZoomFocusComparator>;
using PointGridList = std::vector<Lyli::Calibration::PointGrid>;

cv::Mat estimateCameraMatrix(const Lyli::Image::Metadata::Devices::Lens &lens) {
	double focalLengthPx = (lens.getFocallength() / SENSOR_SIZE) * IMAGE_SIZE;
	double center = IMAGE_SIZE / 2.0;

	cv::Mat cameraMatrix = cv::Mat::zeros(3, 3, CV_64F);
	cameraMatrix.at<double>(0, 0) = focalLengthPx;
	cameraMatrix.at<double>(1, 1) = focalLengthPx;
	cameraMatrix.at<double>(0, 2) = center;
	cameraMatrix.at<double>(1, 2) = center;
	cameraMatrix.at<double>(2, 2) = 1.0;

	return cameraMatrix;
}

double calibrateRotation(const PointGridList &gridList) {
	tbb::combinable<double> sum(0);
	tbb::parallel_for_each(
		gridList,
		[&sum](const auto &grid) {
			// compute rotation of each line
			double angleSum = 0.0;
			for (const auto &line : grid.getHorizontalLines()) {
				// construct a vector of line points
				std::vector<cv::Point2f> linePoints;
				linePoints.reserve(line.line.size());
				for (const auto &point : line.line) {
					linePoints.push_back(point->getPosition());
				}

				// fit line to points
				cv::Vec4f lineParams;
				cv::fitLine(linePoints, lineParams, CV_DIST_L2, 0, 0.01, 0.01);

				cv::Vec2f optimalDir(0.0, 1.0);
				cv::Vec2f lineDir(lineParams[0], lineParams[1]);
				angleSum += std::acos(optimalDir.dot(lineDir));
			}
			sum.local() += angleSum / grid.getHorizontalLines().size();
		}
	);

	return sum.combine(std::plus<double>()) / gridList.size();
}

}

namespace Lyli {
namespace Calibration {

Calibrator::LensConfiguration::LensConfiguration(int zoom, int focus) : zoomStep(zoom), focusStep(focus) {

}

int Calibrator::LensConfiguration::getZoomStep() const {
	return zoomStep;
}

int Calibrator::LensConfiguration::getFocusStep() const {
	return focusStep;
}

class Calibrator::Impl {
public:
	PointGridList pointGridList;
	ClusterMap clusterMap;
	/// Mutex to protect access to pointGridList and clusterMap
	tbb::spin_mutex dataAccessMutex;

	/**
	 * Create target line grid.
	 *
	 * The target line grid is an "optimal" line grid. The calibration
	 * tries to find mapping from the image line grid to the target line grid.
	 */
	PointGrid createTarget(const PointGrid &grid);
};

Calibrator::Calibrator() : pimpl(new Impl) {

}

Calibrator::~Calibrator() {

}

void Calibrator::addGrid(const PointGrid &pointGrid, const Lyli::Image::Metadata &metadata) {
	// thread safe data access
	tbb::spin_mutex::scoped_lock lock(pimpl->dataAccessMutex);

	// store the point grid
	pimpl->pointGridList.push_back(pointGrid);

	// separate grids into clusters based on the lens parameters
	pimpl->clusterMap[metadata.getDevices().getLens()].push_back(pimpl->pointGridList.size() - 1);

	lock.release();
}

std::vector<Calibrator::CalibrationResult> Calibrator::calibrate() {
	// create line grids for all point grids
	std::vector<LineGrid> linegrids;
	int i = 0;
	for(const auto entry : pimpl->pointGridList) {
		linegrids.push_back(LineGrid(entry));
		/*// BEGIN DEBUG
		std::stringstream ss;
		ss << "grid_" << i++ << ".png";
		drawLineGrid(ss.str().c_str(), linegrids.back());
		// END DEBUG*/
	}
	// create a target line grid that is computed as an average of all line grids
	auto target = averageGrids(linegrids);

	/*// BEGIN DEBUG: store the target grid as image
	drawLineGrid("target.png", target.first);
	// END DEBUG*/

	std::vector<CalibrationResult> result;
	result.reserve(pimpl->clusterMap.size());

	// first determine the rotation
	double rotation = calibrateRotation(pimpl->pointGridList);

	// store the results
	for (const auto& cluster : pimpl->clusterMap) {
		cv::Mat cameraMatrix = estimateCameraMatrix(cluster.first);
		cv::Mat distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
		cv::Mat translation = cv::Mat::eye(3, 3, CV_64F);
		CalibrationData calibData(cameraMatrix, distCoeffs, translation, rotation);

		result.push_back(std::make_pair(LensConfiguration(cluster.first.getZoomstep(), cluster.first.getFocusstep()), calibData));
	}
	return result;
}

}
}
