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
#include <unordered_set>

namespace Lyli {
namespace Calibration {

void LineGrid::addPoint(const cv::Point2f& point) {
	// add point to the point storage
	cv::Point2f *stored = storageAdd(point);
	// add point to the horizontal line map
	if (point.y <= CONSTRUCT_LIM) {
		mapAddConstruct(tmpLineMap, point.x, stored);
	}
	else {
		mapAdd(tmpLineMap, point.x, stored);
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
	                        [&](cv::Point2f *point) {this->mapAddConstruct(tmpLineMapOdd, point->y, point);},
	                        [&](cv::Point2f *point) {this->mapAddConstruct(tmpLineMapEven, point->y, point);});
	// add the following points to the lines
	verticalLineConstructor(constructStart, lineMapHorizontal.size(),
	                        [&](cv::Point2f *point) {this->mapAdd(tmpLineMapOdd, point->y, point);},
	                        [&](cv::Point2f *point) {this->mapAdd(tmpLineMapEven, point->y, point);});
	// process the first few lines
	// first, we have to change the keys in the lineMap to the keys of the first point in each line
	// the reason is that we want to use the closest key to the point, but currently the key is for the last point
	for (const auto &entry : tmpLineMapOdd) {
		float key = entry.second.front()->y;
		tmpLineMap.insert(std::make_pair(key, std::move(entry.second)));
	}
	std::swap(tmpLineMapOdd, tmpLineMap);
	tmpLineMap.clear();
	for (const auto &entry : tmpLineMapEven) {
		float key = entry.second.front()->y;
		tmpLineMap.insert(std::make_pair(key, std::move(entry.second)));
	}
	std::swap(tmpLineMapEven, tmpLineMap);
	tmpLineMap.clear();
	// we will process them in reverse, to ensure the closest key is used
	verticalLineConstructor(constructStart, -1,
	                        [&](cv::Point2f *point) {this->mapAdd(tmpLineMapOdd, point->y, point);},
	                        [&](cv::Point2f *point) {this->mapAdd(tmpLineMapEven, point->y, point);});

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

	std::unordered_set<cv::Point2f*> testPoints;
	// remove points that are not in any horizontal line from storage
	for (auto &line : lineMapHorizontal) {
		for (auto &point : *line) {
			testPoints.insert(point);
		}
	}
	for (auto it = storage.begin(); it != storage.end();) {
		if (testPoints.find(it->second.get()) != testPoints.end()) {
			++it;
		}
		else {
			it = storage.erase(it);
		}
	}
	// remove points that are not in any vertical line
	testPoints.clear();
	for (auto &line : lineMapVerticalOdd) {
		for (auto &point : *line) {
			testPoints.insert(point);
		}
	}
	for (auto &line : lineMapVerticalEven) {
		for (auto &point : *line) {
			testPoints.insert(point);
		}
	}
	for (auto &line : lineMapHorizontal) {
		for (auto it = line->begin(); it != line->end();) {
			auto point = *it;
			if (testPoints.find(point) != testPoints.end()) {
				++it;
			}
			else {
				it = line->erase(it);
				storage.erase(point);
			}
		}
	}
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
	cv::Point2f *newPoint = new cv::Point2f(point);
	storage.insert(std::make_pair(newPoint, std::unique_ptr<cv::Point2f>(newPoint)));
	return newPoint;
}

void LineGrid::mapAddConstruct(TmpLineMap &lineMap, float position, cv::Point2f *point) {
	// initial fill - always create a new line
	if (lineMap.empty()) {
		auto res = lineMap.emplace(position, PtrLine());
		// add point to the new line
		res.first->second.push_back(point);
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
	return lineIt->second.push_back(point);
}

void LineGrid::mapAdd(TmpLineMap &lineMap, float position, cv::Point2f *point) {
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
		return lineIt->second.push_back(point);
	}
}

void LineGrid::verticalLineConstructor(int start, int end,
                                       std::function<void(cv::Point2f *)> inserterOdd,
                                       std::function<void(cv::Point2f *)> inserterEven) {
	int step = (end > start) ? 1 : -1;
	for (int i = start; i != end; i+= step) {
		const PtrLine &tmpline = *(lineMapHorizontal.at(i));
		if (i & 1) { // even
			for (const auto &point : tmpline) {
				inserterEven(point);
			}
		}
		else { // odd
			for (const auto &point : tmpline) {
				inserterOdd(point);
			}
		}
	}
}


}
}
