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

#include "linegrid.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <memory>

namespace Lyli {
namespace Calibration {

void LineGrid::addPoint(const cv::Point2f& point) {
	// add point to the horizontal line map
	if (point.y <= CONSTRUCT_LIM) {
		mapAddConstruct(tmpLineMap, point.x, point);
	}
	else {
		mapAdd(tmpLineMap, point.x, point);
	}
}

void LineGrid::finalize() {
	// create a final line map that is used for public interfaces
	lineMapHorizontal = LineMap(tmpLineMap.begin(), tmpLineMap.end());
	tmpLineMap.clear();

	// the points in horizontal lines are sorted implicityl due to the nature
	// of the sweep algorithm

	// execute a sweep algorim to detect vertical lines similar to the one when adding points
	// because the points are already ordered in horizontal lines, we will make use of it

	// construct lines, we will use 6 lines in the first quarter of the image
	// as these should be the most high-quality lines
	TmpLineMap tmpLineMapOdd;
	TmpLineMap tmpLineMapEven;
	std::size_t constructStart = lineMapHorizontal.size() / 4;
	verticalLineConstructor(constructStart, constructStart+6,
	                        [&](const cv::Point2f &point) {this->mapAddConstruct(tmpLineMapOdd, point.y, point);},
	                        [&](const cv::Point2f &point) {this->mapAddConstruct(tmpLineMapEven, point.y, point);});

	// add remaining points to lines
	verticalLineConstructor(0, constructStart,
	                        [&](const cv::Point2f &point) {this->mapAdd(tmpLineMapOdd, point.y, point);},
	                        [&](const cv::Point2f &point) {this->mapAdd(tmpLineMapEven, point.y, point);});
	verticalLineConstructor(constructStart, lineMapHorizontal.size(),
	                        [&](const cv::Point2f &point) {this->mapAdd(tmpLineMapOdd, point.y, point);},
	                        [&](const cv::Point2f &point) {this->mapAdd(tmpLineMapEven, point.y, point);});

	// we must sort the generated lines, as we did not add points to the in order
	// (we first cerated header, then processed points before header and then after)
	for (auto &line : tmpLineMapOdd) {
		std::sort(line.second.begin(), line.second.end(),
		          [](const cv::Point2f *a, const cv::Point2f *b){return a->x < b->x;});
	}
	for (auto &line : tmpLineMapEven) {
		std::sort(line.second.begin(), line.second.end(),
		          [](const cv::Point2f *a, const cv::Point2f *b){return a->x < b->x;});
	}

	// create final line maps that is used for public interfaces
	lineMapVerticalOdd = LineMap(tmpLineMapOdd.begin(), tmpLineMapOdd.end());
	lineMapVerticalEven = LineMap(tmpLineMapEven.begin(), tmpLineMapEven.end());

	// remove points that are not in both horizontal and a vertical line
	// TODO
}

const LineMap& LineGrid::getHorizontalMap() const {
	return lineMapHorizontal;
}

const LineMap& LineGrid::getVerticalMapOdd() const {
	return lineMapVerticalOdd;
}

const LineMap& LineGrid::getVerticalMapEven() const {
	return lineMapVerticalEven;
}

cv::Point2f * LineGrid::storageAdd(const cv::Point2f& point) {
	storage.push_back(std::make_unique<cv::Point2f>(point));
	return storage.back().get();
}

void LineGrid::mapAddConstruct(TmpLineMap &lineMap, float position, const cv::Point2f &point) {
	// initial fill - always create a new line
	if (lineMap.empty()) {
		auto res = lineMap.emplace(position, PtrLine());
		// add point to the new line
		res.first->second.push_back(storageAdd(point));
		return;
	}

	auto ub = lineMap.lower_bound(position);
	auto lb = ub != lineMap.begin() ? std::prev(ub) : lineMap.end();

	float diffLb = lb != lineMap.end() ? std::abs(lb->first - position) : std::numeric_limits<float>::max();
	float diffUb = ub != lineMap.end() ? std::abs(ub->first - position) : std::numeric_limits<float>::max();

	auto lineIt = diffLb < diffUb ? lb : ub;

	// if a point is far from its bounds, it creates a new line
	if(std::abs(lineIt->first - position) > MAX_DIFF) {
		// construct a new line
		auto res = lineMap.emplace(position, PtrLine());
		lineIt = res.first; // TODO: error handling
	}
	else {
		// update the key
		auto line = lineIt ->second;
		auto nextLineIt = lineMap.erase(lineIt);
		lineIt = lineMap.insert(nextLineIt, std::make_pair(position, line));
	}

	// add point to the closest line
	return lineIt->second.push_back(storageAdd(point));
}

void LineGrid::mapAdd(TmpLineMap &lineMap, float position, const cv::Point2f& point) {
	// find
	auto ub = lineMap.lower_bound(position);
	auto lb = ub != lineMap.begin() ? std::prev(ub) : lineMap.end();

	float diffLb = lb != lineMap.end() ? std::abs(lb->first - position) : std::numeric_limits<float>::max();
	float diffUb = ub != lineMap.end() ? std::abs(ub->first - position) : std::numeric_limits<float>::max();

	auto lineIt = diffLb < diffUb ? lb : ub;

	// the points too far from the nearest line are ignored
	if(std::abs(lineIt->first - position) < MAX_DIFF) {
		// update the key
		auto line = lineIt ->second;
		auto nextLineIt = lineMap.erase(lineIt);
		lineIt = lineMap.insert(nextLineIt, std::make_pair(position, line));
		// add point to the closest line
		return lineIt->second.push_back(storageAdd(point));
	}
}

void LineGrid::verticalLineConstructor(std::size_t start, std::size_t end,
                                       std::function<void(const cv::Point2f &)> inserterOdd,
                                       std::function<void(const cv::Point2f &)> inserterEven) {
	for (std::size_t i = start; i < end; ++i) {
		const PtrLine &tmpline = *(lineMapHorizontal.at(i));
		if (i & static_cast<std::size_t>(1)) { // even
			for (const auto &point : tmpline) {
				inserterEven(*point);
			}
		}
		else { // odd
			for (const auto &point : tmpline) {
				inserterOdd(*point);
			}
		}
	}
}


}
}
