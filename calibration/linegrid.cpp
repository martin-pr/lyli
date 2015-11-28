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
#include <unordered_map>

namespace Lyli {
namespace Calibration {

LineGrid::LineGrid() {

}

LineGrid::LineGrid(const LineGrid &other) {
	using PointCopyMap = std::unordered_map<cv::Point2f*, cv::Point2f*>;

	// maps old points to new points
	PointCopyMap pointMap;
	for (const auto &entry : other.storage) {
		cv::Point2f *point = new cv::Point2f(* entry.second.get());
		storage.insert(std::make_pair(point, std::unique_ptr<cv::Point2f>(point)));
		pointMap.insert(std::make_pair(entry.first, point));
	}
	for (const auto &line : other.linesHorizontal) {
		PtrLine newline;
		for (const auto &point : line) {
			newline.push_back(pointMap[point]);
		}
		linesHorizontal.push_back(newline);
	}
	for (const auto &line : other.linesVerticalOdd) {
		PtrLine newline;
		for (const auto &point : line) {
			newline.push_back(pointMap[point]);
		}
		linesVerticalOdd.push_back(newline);
	}
	for (const auto &line : other.linesVerticalEven) {
		PtrLine newline;
		for (const auto &point : line) {
			newline.push_back(pointMap[point]);
		}
		linesVerticalEven.push_back(newline);
	}
}

LineGrid::~LineGrid() {

}

LineGrid &LineGrid::operator=(const LineGrid &other) {
	LineGrid tmp(other);
	std::swap(storage, tmp.storage);
	std::swap(linesHorizontal, tmp.linesHorizontal);
	std::swap(linesVerticalOdd, tmp.linesVerticalOdd);
	std::swap(linesVerticalEven, tmp.linesVerticalEven);
	return *this;
}

void LineGrid::addPoint(const cv::Point2f& point) {
	// add point to the point storage
	cv::Point2f *stored = storageAdd(point);
	// preserve the order
	accumulator.push_back(stored);
}

/*void LineGrid::addPoint(const cv::Point2f& point) {
	// add point to the point storage
	cv::Point2f *stored = storageAdd(point);
	// add point to the horizontal line map
	if (point.y <= CONSTRUCT_LIM) {
		mapAddConstruct(tmpLineMap, point.x, stored);
	}
	else {
		mapAdd(tmpLineMap, point.x, stored);
	}
}*/

void LineGrid::finalize() {
	// temporary line map for horizontal lines
	TmpLineMap tmpLineMap;

	/******************************
	 * construct horizontal lines *
	 ******************************/
	// use the points starting in the first third of the points (which should be ~ 1/3 of the image height)
	// to CONSTRUCT_LIM to construct horizontal lines
	std::size_t constructStart = accumulator.size() / 3;
	float construcStartPos = accumulator[constructStart]->y;
	std::intmax_t i = constructStart;
	for (; accumulator[i]->y < construcStartPos + CONSTRUCT_LIM; ++i) {
		cv::Point2f *point = accumulator[i];
		mapAddConstruct(tmpLineMap, point->x, point);
	}
	// process the following points - just add them to the appropriate lines
	for (; i < static_cast<std::intmax_t>(accumulator.size()); ++i) {
		cv::Point2f *point = accumulator[i];
		mapAdd(tmpLineMap, point->x, point);
	}
	// add preceeding points - this has to be done in reverse
	// first, we have to change the keys in the lineMap to the keys of the first point in each line
	// the reason is that we want to use the closest key to the point, but currently the key is for the last point
	TmpLineMap tmp;
	for (const auto &entry : tmpLineMap) {
		float key = entry.second.front()->x;
		tmp.insert(std::make_pair(key, std::move(entry.second)));
	}
	std::swap(tmpLineMap, tmp);
	tmp.clear();
	// add points
	for (i = constructStart; i >= 0; --i) {
		cv::Point2f *point = accumulator[i];
		mapAdd(tmpLineMap, point->x, point);
	}
	// we must sort the generated lines, as we did not add points to the in order
	// (we first created header, then processed points before header and then after)
	for (auto &line : tmpLineMap) {
		std::sort(line.second.begin(), line.second.end(),
		          [](const cv::Point2f *a, const cv::Point2f *b){return a->y < b->y;});
	}

	/****************************
	 * construct vertical lines *
	 ***************************/

	// create a final line map that is used for public interfaces
	tmpLineMap2LineList(tmpLineMap, linesHorizontal);
	tmpLineMap.clear();

	// the points in horizontal lines are sorted implicitly due to the nature
	// of the sweep algorithm

	// execute a sweep algorithm to detect vertical lines similar to the one when adding points
	// because the points are already ordered in horizontal lines, we will make use of it

	// construct lines, we will use 6 lines in the first third of the image
	// as these should be the most high-quality lines
	TmpLineMap tmpLineMapOdd;
	TmpLineMap tmpLineMapEven;
	constructStart = linesHorizontal.size() / 3;
	verticalLineConstructor(constructStart, constructStart+6,
	                        [&](cv::Point2f *point) {this->mapAddConstruct(tmpLineMapOdd, point->y, point);},
	                        [&](cv::Point2f *point) {this->mapAddConstruct(tmpLineMapEven, point->y, point);});
	// add the following points to the lines
	verticalLineConstructor(constructStart + 6, linesHorizontal.size(),
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
	// (we first created header, then processed points before header and then after)
	for (auto &line : tmpLineMapOdd) {
		std::sort(line.second.begin(), line.second.end(),
		          [](const cv::Point2f *a, const cv::Point2f *b){return a->x < b->x;});
	}
	for (auto &line : tmpLineMapEven) {
		std::sort(line.second.begin(), line.second.end(),
		          [](const cv::Point2f *a, const cv::Point2f *b){return a->x < b->x;});
	}

	// create final line maps that is used for public interfaces
	tmpLineMap2LineList(tmpLineMapOdd, linesVerticalOdd);
	tmpLineMap2LineList(tmpLineMapEven, linesVerticalEven);

	std::unordered_set<cv::Point2f*> testPoints;
	// remove points that are not in any horizontal line from storage
	for (auto &line : linesHorizontal) {
		for (auto &point : line) {
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
	for (auto &line : linesVerticalOdd) {
		for (auto &point : line) {
			testPoints.insert(point);
		}
	}
	for (auto &line : linesVerticalEven) {
		for (auto &point : line) {
			testPoints.insert(point);
		}
	}
	for (auto &line : linesHorizontal) {
		for (auto it = line.begin(); it != line.end();) {
			auto point = *it;
			if (testPoints.find(point) != testPoints.end()) {
				++it;
			}
			else {
				it = line.erase(it);
				storage.erase(point);
			}
		}
	}
}

PtrLineList& LineGrid::getHorizontalLines() {
	return linesHorizontal;
}

const PtrLineList& LineGrid::getHorizontalLines() const {
	return linesHorizontal;
}

PtrLineList& LineGrid::getVerticalLinesOdd() {
	return linesVerticalOdd;
}

const PtrLineList& LineGrid::getVerticalLinesOdd() const {
	return linesVerticalOdd;
}

PtrLineList& LineGrid::getVerticalLinesEven() {
	return linesVerticalEven;
}

const PtrLineList& LineGrid::getVerticalLinesEven() const {
	return linesVerticalEven;
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

	// only the points close enough to the nearest line are added
	if(std::abs(lineIt->first - position) < MAX_DIFF) {
		// update the key
		Lyli::Calibration::PtrLine line = std::move(lineIt->second);
		auto nextLineIt = lineMap.erase(lineIt);
		lineIt = lineMap.emplace_hint(nextLineIt, position, std::move(line));
		// add point to the closest line
		return lineIt->second.push_back(point);
	}
}

void LineGrid::tmpLineMap2LineList(const TmpLineMap &map, PtrLineList &list) {
	for (auto entry : map) {
		list.push_back(entry.second);
	}
}

void LineGrid::verticalLineConstructor(int start, int end,
                                       std::function<void(cv::Point2f *)> inserterOdd,
                                       std::function<void(cv::Point2f *)> inserterEven) {
	int step = (end > start) ? 1 : -1;
	for (int i = start; i != end; i+= step) {
		const PtrLine &tmpline = linesHorizontal[i];
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
