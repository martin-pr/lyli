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
#include "mathutil.h"
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

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <tbb/concurrent_vector.h>
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

/**
 * Find line parameters for line that best fits a line of points.
 *
 * \param line line formed by a set of points
 * \return (vx, vy, x0, y0), where (vx, vy) is a normalized vector collinear to the line and (x0, y0) is a point on the line.
 */
cv::Vec4f findLineParams(const Lyli::Calibration::PointGrid::Line& line) {
	// construct a vector of line points
	std::vector<cv::Point2f> linePoints;
	linePoints.reserve(line.line.size());
	for (const auto &point : line.line) {
		linePoints.push_back(point->getPosition());
	}

	// fit line to points
	cv::Vec4f lineParams;
	cv::fitLine(linePoints, lineParams, CV_DIST_L2, 0, 0.01, 0.001);

	return lineParams;
}

/**
 * Rotate line in general form by a given angle
 *
 * \see http://stackoverflow.com/questions/27442437/rotate-a-line-by-a-given-angle
 *
 * \param line line to rotate
 * \param angle rotation angle
 * \return rotated line
 */
cv::Vec3f rotateGeneralLine(cv::Vec3f line, double angle) {
	double theta = std::atan2(line[1], line[0]);
	double p = -line[2]/std::sqrt(line[0]*line[0] + line[1]*line[1]);
	return cv::Vec3f(std::cos(theta + angle), std::sin(theta + angle), -p);
}

/**
 * Convert the openCV parametric line form to the general form.
 *
 * \param parametric parametric form of line as (vx, vy, x0, y0)
 * \return coefficients (a,b,c) of the general form ax+by+c=0
 */
cv::Vec3f parametricToGeneral(cv::Vec4f parametric) {
	// the general form is computed as
	//   vy*x - vx*y + (y0*vx - x0*vy) = 0
	auto d = parametric[3]*parametric[0] - parametric[2]*parametric[1];
	return cv::Vec3f(parametric[1], -parametric[0], d);
}

double calibrateRotation(const PointGridList &gridList) {
	tbb::concurrent_vector<double> angles;
	angles.reserve(gridList.size());
	tbb::parallel_for_each(
		gridList,
		[&angles](const auto &grid) {
			// compute rotation of each line
			std::vector<double> localAngles;
			localAngles.reserve(grid.getVerticalLines().size());
			for (const auto &line : grid.getVerticalLines()) {
				cv::Vec4f lineParams(findLineParams(line));

				cv::Vec2d optimalDir(1.0, 0.0);
				cv::Vec2d lineDir(lineParams[0], lineParams[1]);
				localAngles.push_back(std::acos(optimalDir.dot(lineDir)));
			}
			angles.push_back(Lyli::Calibration::filteredAverage(localAngles, 2.0));
		}
	);

	return Lyli::Calibration::filteredAverage(angles, 2.0);
}

/**
 * \param direction a unit direction vector corresponding to the general tranlation direction we're computing
 *        Eg. if we are computing vertical translation (ie. translation in the x-direction due to swapped x and y)
 *        the direction should be (1, 0)
 * \param angle the rotation of the image that is applied prior the computation
 */
double findTranslation(const Lyli::Calibration::PointGrid::LineList &lines,
                       cv::Vec2f direction, double angle,
                       const Lyli::Calibration::LineGrid &target, const Lyli::Calibration::GridMapper &mapper) {
	std::vector<double> distances;
	distances.reserve(lines.size());
	for (const auto &line : lines) {
		// fit a line to points and rotate it by the given angle
		cv::Vec4f parametric(findLineParams(line));
		cv::Vec3f general(parametricToGeneral(parametric));
		cv::Vec3f lineParams(rotateGeneralLine(general, angle));

		// find the corresponding line in target grid
		Lyli::Calibration::LineGrid::Line targetLine;
		if (direction.dot(cv::Vec2f(1, 0)) > 0.5) {
			targetLine = target.getHorizontalLines()[mapper.mapHorizontal(line.line.front()->getHorizontalLineIndex())];
		}
		else {
			targetLine = target.getVerticalLines()[mapper.mapVertical(line.line.front()->getVerticalLineIndex())];
		}

		// find the general form of the target line, where:
		//   the normal=direction, and the translation corresponds to the line position
		cv::Vec3f targetParams(direction[0], direction[1], -targetLine.position);

		// find the general form of the intersection line that is orthogonal to the target line
		// and which goes through the middle of the image
		cv::Vec3f intersectLineParams(direction[1], -direction[0], -IMAGE_SIZE/2);

		// find the intersection points between the intersection line and other lines
		// we use the homogenous coordinates and duality for this
		cv::Vec3f point1(lineParams.cross(intersectLineParams));
		cv::Vec3f point2(targetParams.cross(intersectLineParams));

		// find the required translation as the distance between the two points
		// note that point1 and point2 are still in homogenous coordinates
		cv::Vec2f point1E2(point1[0]/point1[2], point1[1]/point1[2]);
		cv::Vec2f point2E2(point2[0]/point2[2], point2[1]/point2[2]);
		cv::Vec2f dif(point1E2 - point2E2);
		// the norm is always unsigned, meaning we have no directional information
		// this provides correct sign describing the general direction of the translation
		float sign = Lyli::Calibration::sgn(dif.dot(cv::Vec2f(1.0, 1.0)));
		distances.push_back(sign * cv::norm(dif));
	}
	return Lyli::Calibration::filteredAverage(distances, 2.0);
}

cv::Vec2f calibrateTranslation(const PointGridList &gridList, double angle,
                               const Lyli::Calibration::LineGrid &target, const std::vector<Lyli::Calibration::GridMapper> &mappers) {
	std::vector<double> verticalDistances;
	verticalDistances.reserve(gridList.size());
	std::vector<double> horizontalDistances;
	horizontalDistances.reserve(gridList.size());
	for (std::size_t i = 0; i < gridList.size(); ++i) {
		verticalDistances.push_back(findTranslation(gridList[i].getHorizontalLines(), cv::Vec2f(1, 0), -angle, target, mappers[i]));
		horizontalDistances.push_back(findTranslation(gridList[i].getVerticalLines(), cv::Vec2f(0, 1), -angle, target, mappers[i]));
	}
	float vertical = Lyli::Calibration::filteredAverage(verticalDistances, 2.0);
	float horizontal = Lyli::Calibration::filteredAverage(horizontalDistances, 2.0);

	return {vertical, horizontal};
}

}

namespace Lyli {
namespace Calibration {

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

CalibrationData Calibrator::calibrate() {
	// create line grids for all point grids
	std::vector<LineGrid> linegrids;
	int i = 0;
	for (const auto entry : pimpl->pointGridList) {
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

	// the lens array calibration
	double rotation = calibrateRotation(pimpl->pointGridList);
	cv::Vec2f translation = calibrateTranslation(pimpl->pointGridList, -rotation, target.first, target.second);
	ArrayParameters arrayCalib(target.first, translation, rotation);

	// the lens calibration depending on the zoom etc.
	CalibrationData::LensCalibration lensCalib;
	for (const auto& cluster : pimpl->clusterMap) {
		cv::Mat cameraMatrix = estimateCameraMatrix(cluster.first);
		cv::Mat distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
		LensParameters lensParam(cameraMatrix, distCoeffs);
		lensCalib.push_back(std::make_pair(LensConfiguration(cluster.first.getZoomstep(), cluster.first.getFocusstep()), lensParam));
	}

	return CalibrationData(arrayCalib, lensCalib);
}

}
}
