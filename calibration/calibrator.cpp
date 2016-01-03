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

// BEGIN DEBUG
#include <sstream>
#include <opencv/highgui.h>
// END DEBUG

namespace {

void drawLineGrid(const char* file, const Lyli::Calibration::LineGrid &lineGrid) {
	static constexpr auto SIZE = 3280;

	cv::Mat dst(SIZE, SIZE, CV_8UC3);
	dst = cv::Scalar(256, 256, 256);
	const auto &linesHorizontal = lineGrid.getHorizontalLines();
	for (const auto &line : linesHorizontal) {
		cv::Scalar color = line.subgrid == Lyli::Calibration::SubGrid::SUBGRID_A ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 255, 0);
		cv::line(dst, cv::Point2f(0, line.position), cv::Point2f(SIZE, line.position), color);
	}
	const auto &linesVertical = lineGrid.getVerticalLines();
	for (const auto &line : linesVertical) {
		cv::Scalar color = line.subgrid == Lyli::Calibration::SubGrid::SUBGRID_A ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 255, 0);
		cv::line(dst, cv::Point2f(line.position, 0), cv::Point2f(line.position, SIZE), color);
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

/**
 * Calibrate a single cluster
 * @param gridList list of all available point grids
 * @param cluster list with indices to the gridList selecting the images to process
 * @param target desired target grid
 * @param mappers mappers that maps line indices from the gridList to lines in target
 */
Lyli::Calibration::CalibrationData calibrateCluster(const PointGridList &gridList, const Cluster &cluster,
                                                    const Lyli::Calibration::LineGrid &target, const std::vector<Lyli::Calibration::GridMapper> &mappers) {

	// use the opencv lens calibration
	// the object points (source, in 3D)
	std::vector<std::vector<cv::Point3f>> objectPoints;
	// the image points (destination, in 2D)
	std::vector<std::vector<cv::Point2f>> imagePoints;
	for (const auto &index : cluster) {
		objectPoints.push_back(std::vector<cv::Point3f>());
		imagePoints.push_back(std::vector<cv::Point2f>());
		for (const auto &line : gridList[index].getHorizontalLines()) {
			for (const auto &point : line.line) {
				// the object points are taken directly from grid list
				objectPoints.back().push_back(cv::Point3f(point->getPosition().x, point->getPosition().y, 0.0));
				// the image points are found by looking up the lines on whose intersection the point lies
				// and then finding the corresponding lines in the target image and computing their instersection
				int srcIndexY = point->getHorizontalLineIndex();
				int srcIndexX = point->getVerticalLineIndex();
				auto idxy = mappers[index].mapHorizontal(srcIndexY);
				auto idxx = mappers[index].mapVertical(srcIndexX);
				auto y = target.getHorizontalLines()[idxy].position;
				auto x = target.getVerticalLines()[idxx].position;
				imagePoints.back().push_back(cv::Point2f(x, y));
			}
		}
	}
	// remaining parameters
	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
	cv::Mat distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
	std::vector<cv::Mat> rvecs, tvecs;
	std::vector<float> reprojErrs;

	// run the calibration
	cv::calibrateCamera(objectPoints, imagePoints, cv::Size(3280, 3280), cameraMatrix, distCoeffs, rvecs, tvecs);

	// get the 2D affine transform
	cv::Mat rotation3D;
	cv::Rodrigues(rvecs[0], rotation3D); // get 3D rotation
	cv::Mat rotation = cv::Mat::eye(3, 3, CV_64F);
	cv::Mat editRot = rotation(cv::Range(0, 2), cv::Range(0, 2));
	rotation3D(cv::Range(0, 2), cv::Range(0, 2)).copyTo(editRot); // we we need 2D, so we set z = 0 => top left submatrix suffice

	cv::Mat translation = cv::Mat::eye(3, 3, CV_64F);
	cv::Mat editTransl = translation.col(2).rowRange(0, 2);
	tvecs[0].rowRange(0, 2).copyTo(editTransl);

	return Lyli::Calibration::CalibrationData(cameraMatrix, distCoeffs, translation, rotation);
}

}

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

	// store the point grid
	pimpl->pointGridList.push_back(pointGrid);

	// separate grids into clusters based on the lens parameters
	pimpl->clusterMap[metadata.getDevices().getLens()].push_back(pimpl->pointGridList.size() - 1);
}

Calibrator::CalibrationResult Calibrator::calibrate() {
	// create line grids for all point grids
	std::vector<LineGrid> linegrids;
	int i = 0;
	for(const auto entry : pimpl->pointGridList) {
		linegrids.push_back(LineGrid(entry));
		// BEGIN DEBUG
		std::stringstream ss;
		ss << "grid_" << i++ << ".png";
		drawLineGrid(ss.str().c_str(), linegrids.back());
		// END DEBUG
	}
	// create a target line grid that is computed as an average of all line grids
	auto target = averageGrids(linegrids);

	// BEGIN DEBUG: store the target grid as image
	drawLineGrid("target.png", target.first);
	// END DEBUG

	CalibrationResult res;
	for (const auto &cluster : pimpl->clusterMap) {
		// use only cluster that have more than one image for calibration to avoid stability issues
		if(cluster.second.size() > 1) {
			res.push_back(std::make_pair(LensConfiguration(cluster.first.getZoomstep(), cluster.first.getFocusstep()),
			              calibrateCluster(pimpl->pointGridList, cluster.second, target.first, target.second)));
		}
	}
	return res;
}

}
}
