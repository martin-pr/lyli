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

#include "lensfilter.h"

#include "linegrid.h"

#include <cmath>
#include <utility>

#ifndef NDEBUG
#include <iostream>
#endif

/**
 * \WARNING Due to the fact ht elines are detected in a rotated image, the
 * point corrdinates are swapped, ie.
 *  -> the lines are horizontal
 *  -> but the horizontal coordinate is y instead of x
 */

namespace {

template <typename LineType, typename LambdaOdd, typename LambdaEven>
void applyAlternating(LineType lines, LambdaOdd funcOdd, LambdaEven funcEven) {
	const std::size_t evenSize = lines.size() % 2 == 0 ? lines.size() : lines.size() - 1;
	const std::size_t iterations = evenSize / 2;
	auto it = lines.begin();
	for (std::size_t i = 0; i < iterations; ++i) {
		funcOdd(it++);
		funcEven(it++);
	}
	if (lines.size() - evenSize != 0) {
		funcOdd(it++);
	}
}

/**
 * Represents a boundary edge of the image.
 *
 * The edge is fitted using quadratic function as the barrel distortion and
 * pincushion distortion are quadratic.
 */
class BoundaryEdge {
public:
	BoundaryEdge()
		: m_a(0.0), m_b(0.0), m_c(0.0) {

	}

	BoundaryEdge(double a, double b, double c)
		: m_a(a), m_b(b), m_c(c) {

	}

	double interpolate(double val) const {
		return val*(m_a*val + m_b) + m_c;
	}

private:
	double m_a;
	double m_b;
	double m_c;
};

using LinePtrList = std::vector<Lyli::Calibration::Line*>;
using DPair = std::pair<double, double>;
using ImgEdge = std::pair<BoundaryEdge, BoundaryEdge>;
using ImgBoundary = std::pair<ImgEdge, ImgEdge>;

/**
 * Compute the average line length.
 *
 * \param lines lines for which the average is computed
 * \return pair <odd averafe, even average>
 */
DPair averageLineLen(const ::Lyli::Calibration::PointGrid& lines) {
	double sumOdd = 0.0;
	double sumEven = 0.0;

	applyAlternating(lines,
	                 [&sumOdd](const decltype(lines.begin()) &lineit){sumOdd += lineit->second.size();},
	                 [&sumEven](const decltype(lines.begin()) &lineit){sumEven += lineit->second.size();});

	const std::size_t evenSize = lines.size() % 2 == 0 ? lines.size() : lines.size() - 1;
	const std::size_t iterations = evenSize / 2;
	double countOdd = iterations + (lines.size() - evenSize);

	return std::make_pair(sumOdd / countOdd, sumEven / iterations);
}

/**
 * Filter out lines that are too short
 *
 * \param lines lines to filter
 * \param avgLen average lengths of lines
 * \param filteredMap OUT filtered map of lines
 * \param filteredList OUT list of pointers to filtered lines
 */
int shortLineFilter(const Lyli::Calibration::LineMap& lines, DPair avgLen, Lyli::Calibration::LineMap &filteredMap, LinePtrList &filteredList) {
	// use only one average to speedup the proces, both averages should be pretty close anyway
	double avg = 0.5 * (avgLen.first + avgLen.second);

	for (const auto &line : lines) {
		if (std::abs(line.second.size() - avg) < 7) {
			auto res = filteredMap.insert(line);
			filteredList.push_back(& (res.first->second));
		}
	}

	return lines.size() - filteredList.size();
}

/**
 * Compute average distance between lens centroid for odd and even lines
 */
DPair averageLensDist(const LinePtrList &filteredList) {
	// the middle two lines are used to compute the average
	std::size_t posOdd = (filteredList.size() / 2) & 1u ? (filteredList.size() / 2) - 1 : (filteredList.size() / 2);
	const auto &lineOdd(* filteredList[posOdd]);
	const auto &lineEven(* filteredList[posOdd + 1]);
	double sumOdd = 0.0;
	double sumEven = 0.0;

	for (std::size_t i = 1; i < lineOdd.size(); ++i) {
		sumOdd += lineOdd[i].y - lineOdd[i-1].y;
	}
	for (std::size_t i = 1; i < lineEven.size(); ++i) {
		sumEven += lineEven[i].y - lineEven[i-1].y;
	}

	return std::make_pair(sumOdd / (lineOdd.size() - 1.0), sumEven / (lineEven.size() - 1.0));
}

/**
 * Compute the coefficients of the boundary by fitting quadratic function using least squares.
 *
 * The least squares are computed using the Gram–Schmidt process
 */
BoundaryEdge leastSquaresBoundary(const ::Lyli::Calibration::Line &points) {
	cv::Mat ma(3, points.size(), CV_64FC1);
	cv::Mat mb(1, points.size(), CV_64FC1);
	double* a1 = ma.ptr<double>(0);
	double* a2 = ma.ptr<double>(1);
	double* a3 = ma.ptr<double>(2);
	double* b = mb.ptr<double>(0);

	std::size_t i = 0;
	for (auto point : points) {
		a1[i] = point.x * point.x;
		a2[i] = point.x;
		a3[i] = 1.0;
		b[i] = point.y;
		++i;
	}

	cv::Mat ma0(3, 3, CV_64FC1);
	for (i = 0; i < 3; ++i) {
		for (std::size_t j = 0; j < 3; ++j) {
			ma0.at<double>(i, j) = ma.row(i).dot(ma.row(j));
		}
	}

	cv::Mat mb0(ma * mb.t());

	// solve ma0 * x = b0;
	cv::Mat x(ma0.inv() * mb0);

	return BoundaryEdge(x.at<double>(0, 0), x.at<double>(1, 0), x.at<double>(2, 0));
}

/**
 * Compute coeficients of an image boundary.
 *
 * \return pair \<LeftEdge,RightEdge\>, where each edge is a pair \<Odd,Even\>
 */
ImgBoundary computeBound(const LinePtrList &lines) {
	::Lyli::Calibration::Line leftOdd;
	::Lyli::Calibration::Line leftEven;
	::Lyli::Calibration::Line rightOdd;
	::Lyli::Calibration::Line rightEven;

	applyAlternating(lines,
	                 [&leftOdd,&rightOdd](const decltype(lines.begin()) &lineit){
	                 	leftOdd.push_back((*lineit)->front());
	                 	rightOdd.push_back((*lineit)->back());
	                 },
	                 [&leftEven,&rightEven](const decltype(lines.begin()) &lineit){
	                 	leftEven.push_back((*lineit)->front());
	                 	rightEven.push_back((*lineit)->back());
	                 });

	ImgEdge left(leastSquaresBoundary(leftOdd), leastSquaresBoundary(leftEven));
	ImgEdge right(leastSquaresBoundary(rightOdd), leastSquaresBoundary(rightEven));

	return std::make_pair(left, right);
}

/**
 * Add missing boundary points.
 */
void repairBounds(const LinePtrList &lines, ImgBoundary boundary, DPair avgLensDist) {
	const ImgEdge &edgeLeft = boundary.first;
	const ImgEdge &edgeRight = boundary.second;
	const double maxLensDistOdd = 0.5 * avgLensDist.first;
	const double maxLensDistEven = 0.5 * avgLensDist.second;
	applyAlternating(lines,
	                 [&](const decltype(lines.begin()) &lineit){
	                 	::Lyli::Calibration::Line &line = **lineit;
	                 	double expectedL = edgeLeft.first.interpolate(line.front().x);
	                 	double expectedR = edgeRight.first.interpolate(line.back().x);
	                 	if (std::abs(line.front().y - expectedL) > maxLensDistOdd) {
	                 		line.insert(line.begin(), cv::Point2f(line.front().x, expectedL));
	                 	}
	                 	if (std::abs(line.back().y - expectedR) > maxLensDistOdd) {
	                 		line.push_back(cv::Point2f(line.back().x, expectedR));
	                 	}
	                 },
	                 [&](const decltype(lines.begin()) &lineit){
	                 	::Lyli::Calibration::Line &line = **lineit;
	                 	double expectedL = edgeLeft.second.interpolate(line.front().x);
	                 	double expectedR = edgeRight.second.interpolate(line.back().x);
	                 	if (std::abs(line.front().y - expectedL) > maxLensDistEven) {
	                 		line.insert(line.begin(), cv::Point2f(line.front().x, expectedL));
	                 	}
	                 	if (std::abs(line.back().y - expectedR) > maxLensDistEven) {
	                 		line.push_back(cv::Point2f(line.back().x, expectedR));
	                 	}
	                 });
}

}

namespace Lyli {
namespace Calibration {

LineMap LensFilter::filter(const LineMap& lines) {
	// compute average count of points in lines
	DPair avgLen = averageLineLen(lines);

	// remove the lines that are too short
	LineMap filtered;
	LinePtrList filteredList;
	shortLineFilter(lines, avgLen, filtered, filteredList);

	// find the left and the right boundary
	ImgBoundary boundary = computeBound(filteredList);

	// now compute average distance between lenses for odd and even lines,
	// it will be used to detect and add missing centroids
	DPair avgLensDist = averageLensDist(filteredList);

	// ensure all boundary pixels are detected and fill the missing ones using interpolation
	// if needed
	repairBounds(filteredList, boundary, avgLensDist);

	// now go through all lines and detect and missing centroids, possibly also remove outliers
	// TODO

	return filtered;
}

}
}
