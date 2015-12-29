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

#include "pointgrid.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <memory>
#include <unordered_set>
#include <unordered_map>

namespace Lyli {
namespace Calibration {

/*********
 * Point *
 *********/

PointGrid::Point::Point(const cv::Point2f &pos) : position(pos), horizontalLine(nullptr), verticalLine(nullptr) {

}

PointGrid::Point::~Point() {

}

PointGrid::Point::Point(const Point &point) : position(point.position), horizontalLine(point.horizontalLine), verticalLine(point.verticalLine) {

}

PointGrid::Point& PointGrid::Point::operator=(const Point &point) {
	if (this != &point) {
		Point tmp(point);
		std::swap(tmp, *this);
	}
	return *this;
}

const cv::Point2f& PointGrid::Point::getPosition() const {
	return position;
}

const PointGrid::Line* PointGrid::Point::getHorizontalLine() const {
	return horizontalLine;
}

const PointGrid::Line* PointGrid::Point::getVerticalLine() const {
	return verticalLine;
}

/*************
 * PointGrid *
 *************/

PointGrid::PointGrid() {

}

PointGrid::PointGrid(const PointGrid &other) {
	using PointCopyMap = std::unordered_map<Point*, Point*>;

	// maps old points to new points
	PointCopyMap pointMap;
	for (const auto &entry : other.storage) {
		auto *point = new Point(* entry.second.get());
		storage.insert(std::make_pair(point, std::unique_ptr<Point>(point)));
		pointMap.insert(std::make_pair(entry.first, point));
	}
	for (const auto &line : other.linesHorizontal) {
		linesHorizontal.emplace_back();
		auto &newLine(linesHorizontal.back());
		for (const auto &point : line) {
			Point* newPoint(pointMap[point]);
			newPoint->horizontalLine = &newLine;
			newLine.push_back(newPoint);
		}
	}
	for (const auto &line : other.linesVerticalOdd) {
		linesVerticalOdd.emplace_back();
		auto &newLine(linesHorizontal.back());
		for (const auto &point : line) {
			Point* newPoint(pointMap[point]);
			newPoint->verticalLine = &newLine;
			newLine.push_back(newPoint);
		}
	}
	for (const auto &line : other.linesVerticalEven) {
		linesHorizontal.emplace_back();
		auto &newLine(linesHorizontal.back());
		for (const auto &point : line) {
			Point* newPoint(pointMap[point]);
			newPoint->verticalLine = &newLine;
			newLine.push_back(newPoint);
		}
	}
}

PointGrid::~PointGrid() {

}

PointGrid &PointGrid::operator=(const PointGrid &other) {
	PointGrid tmp(other);
	std::swap(storage, tmp.storage);
	std::swap(linesHorizontal, tmp.linesHorizontal);
	std::swap(linesVerticalOdd, tmp.linesVerticalOdd);
	std::swap(linesVerticalEven, tmp.linesVerticalEven);
	return *this;
}

void PointGrid::addPoint(const cv::Point2f& point) {
	// add point to the point storage
	Point *stored = storageAdd(point);
	// preserve the order
	accumulator.push_back(stored);
}

void PointGrid::finalize() {
	// temporary line map for horizontal lines
	TmpLineMap tmpLineMap;

	/******************************
	 * construct horizontal lines *
	 ******************************/
	// use the points starting in the first third of the points (which should be ~ 1/3 of the image height)
	// to CONSTRUCT_LIM to construct horizontal lines
	std::size_t constructStart = accumulator.size() / 3;
	float construcStartPos = accumulator[constructStart]->getPosition().y;
	std::intmax_t i = constructStart;
	for (; accumulator[i]->getPosition().y < construcStartPos + CONSTRUCT_LIM; ++i) {
		auto *point = accumulator[i];
		mapAddConstruct(tmpLineMap, point->getPosition().x, point);
	}
	// process the following points - just add them to the appropriate lines
	for (; i < static_cast<std::intmax_t>(accumulator.size()); ++i) {
		auto *point = accumulator[i];
		mapAdd(tmpLineMap, point->getPosition().x, point);
	}
	// add preceeding points - this has to be done in reverse
	// first, we have to change the keys in the lineMap to the keys of the first point in each line
	// the reason is that we want to use the closest key to the point, but currently the key is for the last point
	TmpLineMap tmp;
	for (const auto &entry : tmpLineMap) {
		float key = entry.second.front()->getPosition().x;
		tmp.insert(std::make_pair(key, std::move(entry.second)));
	}
	std::swap(tmpLineMap, tmp);
	tmp.clear();
	// add points
	for (i = constructStart; i >= 0; --i) {
		auto *point = accumulator[i];
		mapAdd(tmpLineMap, point->getPosition().x, point);
	}
	// we must sort the generated lines, as we did not add points to the in order
	// (we first created header, then processed points before header and then after)
	for (auto &line : tmpLineMap) {
		std::sort(line.second.begin(), line.second.end(),
		          [](const Point *a, const Point *b){return a->getPosition().y < b->getPosition().y;});
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
	                        [&](Point *point) {this->mapAddConstruct(tmpLineMapOdd, point->getPosition().y, point);},
	                        [&](Point *point) {this->mapAddConstruct(tmpLineMapEven, point->getPosition().y, point);});
	// add the following points to the lines
	verticalLineConstructor(constructStart + 6, linesHorizontal.size(),
	                        [&](Point *point) {this->mapAdd(tmpLineMapOdd, point->getPosition().y, point);},
	                        [&](Point *point) {this->mapAdd(tmpLineMapEven, point->getPosition().y, point);});
	// process the first few lines
	// first, we have to change the keys in the lineMap to the keys of the first point in each line
	// the reason is that we want to use the closest key to the point, but currently the key is for the last point
	for (const auto &entry : tmpLineMapOdd) {
		float key = entry.second.front()->getPosition().y;
		tmpLineMap.insert(std::make_pair(key, std::move(entry.second)));
	}
	std::swap(tmpLineMapOdd, tmpLineMap);
	tmpLineMap.clear();
	for (const auto &entry : tmpLineMapEven) {
		float key = entry.second.front()->getPosition().y;
		tmpLineMap.insert(std::make_pair(key, std::move(entry.second)));
	}
	std::swap(tmpLineMapEven, tmpLineMap);
	tmpLineMap.clear();
	// we will process them in reverse, to ensure the closest key is used
	verticalLineConstructor(constructStart, -1,
	                        [&](Point *point) {this->mapAdd(tmpLineMapOdd, point->getPosition().y, point);},
	                        [&](Point *point) {this->mapAdd(tmpLineMapEven, point->getPosition().y, point);});

	// we must sort the generated lines, as we did not add points to the in order
	// (we first created header, then processed points before header and then after)
	for (auto &line : tmpLineMapOdd) {
		std::sort(line.second.begin(), line.second.end(),
		          [](const Point *a, const Point *b){return a->getPosition().x < b->getPosition().x;});
	}
	for (auto &line : tmpLineMapEven) {
		std::sort(line.second.begin(), line.second.end(),
		          [](const Point *a, const Point *b){return a->getPosition().x < b->getPosition().x;});
	}

	// create final line maps that is used for public interfaces
	tmpLineMap2LineList(tmpLineMapOdd, linesVerticalOdd);
	tmpLineMap2LineList(tmpLineMapEven, linesVerticalEven);

	// set of all points in horizontal lines
	std::unordered_set<Point*> testPoints;
	for (auto &line : linesHorizontal) {
		for (auto &point : line) {
			testPoints.insert(point);
		}
	}
	// remove points that are not in any horizontal line from storage
	// beucase the points are not in any horizontal line, they can't in any vertical line either
	for (auto it = storage.begin(); it != storage.end();) {
		if (testPoints.find(it->second.get()) != testPoints.end()) {
			++it;
		}
		else {
			it = storage.erase(it);
		}
	}
	// remove points that are not in any vertical line (these has to be removed from the horizontal line, too)
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

	// setup links from points to the corresponding lines
	for (auto &line : linesVerticalOdd) {
		for (auto &point : line) {
			point->verticalLine = &line;
		}
	}
	for (auto &line : linesVerticalEven) {
		for (auto &point : line) {
			point->verticalLine = &line;
		}
	}
	for (auto &line : linesHorizontal) {
		for (auto &point : line) {
			point->horizontalLine = &line;
		}
	}
}

const PointGrid::LineList& PointGrid::getHorizontalLines() const {
	return linesHorizontal;
}

const PointGrid::LineList& PointGrid::getVerticalLinesOdd() const {
	return linesVerticalOdd;
}

const PointGrid::LineList& PointGrid::getVerticalLinesEven() const {
	return linesVerticalEven;
}

PointGrid::Point* PointGrid::storageAdd(const cv::Point2f& point) {
	Point *newPoint(new Point(point));
	storage.insert(std::make_pair(newPoint, std::unique_ptr<Point>(newPoint)));
	return newPoint;
}

void PointGrid::mapAddConstruct(TmpLineMap &lineMap, float position, Point *point) {
	// always create a new line when first point is added
	if (lineMap.empty()) {
		auto res = lineMap.emplace(position, Line());
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
		auto res = lineMap.emplace(position, Line());
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

void PointGrid::mapAdd(TmpLineMap &lineMap, float position, Point *point) {
	// find
	auto ub = lineMap.lower_bound(position);
	auto lb = ub != lineMap.begin() ? std::prev(ub) : lineMap.end();

	float diffLb = lb != lineMap.end() ? std::abs(lb->first - position) : std::numeric_limits<float>::max();
	float diffUb = ub != lineMap.end() ? std::abs(ub->first - position) : std::numeric_limits<float>::max();

	auto lineIt = diffLb < diffUb ? lb : ub;

	// only the points close enough to the nearest line are added
	if(std::abs(lineIt->first - position) < MAX_DIFF) {
		// update the key
		Line line(std::move(lineIt->second));
		auto nextLineIt = lineMap.erase(lineIt);
		lineIt = lineMap.emplace_hint(nextLineIt, position, std::move(line));
		// add point to the closest line
		return lineIt->second.push_back(point);
	}
}

void PointGrid::tmpLineMap2LineList(const TmpLineMap &map, LineList &list) {
	for (auto entry : map) {
		list.push_back(entry.second);
	}
}

void PointGrid::verticalLineConstructor(int start, int end,
                                       std::function<void(Point *)> inserterOdd,
                                       std::function<void(Point *)> inserterEven) {
	int step = (end > start) ? 1 : -1;
	for (int i = start; i != end; i+= step) {
		const Line &tmpline = linesHorizontal[i];
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
