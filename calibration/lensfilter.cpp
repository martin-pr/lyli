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

#include <cmath>
#include <utility>

#ifndef NDEBUG
#include <iostream>
#endif

using LinePtrList = std::vector<Lyli::Calibration::Line*>;

namespace Lyli {
namespace Calibration {

LineMap LensFilter::filter(const LineMap& lines) {
#ifndef NDEBUG
	struct {
		std::size_t shortlines = 0;
		std::size_t boundary = 0;
	} stats;
#endif

	// compute average count of points in lines
	double sum = 0.0;
	for (const auto &line : lines) {
		sum += line.second.size();
	}
	double avg = sum / lines.size();

	// remove the lines that are too short
	LineMap filtered;
	LinePtrList filteredList;
	for (const auto &line : lines) {
		if (std::abs(line.second.size() - avg) < 7) {
			auto res = filtered.insert(line);
			filteredList.push_back(& (res.first->second));
		}
	}

#ifndef NDEBUG
	stats.shortlines = lines.size() - filteredList.size();
#endif

	// now compute average distance between lenses for odd and even lines,
	// it will be used to detect and add missing centroids
	// the middle two lines are used to compute the average
	std::size_t posOdd = !((filteredList.size() / 2) & 1u) ? (filteredList.size() / 2) : (filteredList.size() / 2) - 1;
	const Line &lineOdd(* filteredList[posOdd]);
	const Line &lineEven(* filteredList[posOdd + 1]);
	double sumOdd = 0.0;
	double sumEven = 0.0;
	for (std::size_t i = 1; i < lineOdd.size(); ++i) {
		sumOdd += lineOdd[i].y - lineOdd[i-1].y;
	}
	for (std::size_t i = 1; i < lineEven.size(); ++i) {
		sumEven += lineEven[i].y - lineEven[i-1].y;
	}
	double avgOdd = sumOdd / (lineOdd.size() - 1.0);
	double avgEven = sumEven / (lineEven.size() - 1.0);

	// find the average position of the boundary pixels
	std::pair<double, double> boundsSumOdd = std::make_pair(0.0, 0.0);
	std::pair<double, double> boundsSumEven = std::make_pair(0.0, 0.0);
	bool isOdd = true;
	std::size_t oddNumber = 0;
	for (const auto &line : filtered) {
		if (isOdd) {
			boundsSumOdd.first += line.second.front().y;
			boundsSumOdd.second += line.second.back().y;
			++oddNumber;
		}
		else {
			boundsSumEven.first += line.second.front().y;
			boundsSumEven.second += line.second.back().y;
		}
		isOdd = !isOdd;
	}
	std::pair<double, double> boundsOdd = std::make_pair(boundsSumOdd.first / oddNumber, boundsSumOdd.second / oddNumber);
	std::pair<double, double> boundsEven = std::make_pair(boundsSumEven.first / (filtered.size() - oddNumber), boundsSumEven.second / (filtered.size() - oddNumber));

	// add the boundary pixels (first and last pixel in each line) if they are missing
	// special handling of the first two lines
	for (std::size_t i = 0; i < 2; ++i) {
		Line &line = *filteredList[i];
		double avgDiff = (i&1u) ? avgEven : avgOdd;
		std::pair<double, double> bounds = (i&1u) ? boundsEven : boundsOdd;
		if (std::abs(line.front().y - bounds.first) > 0.5*avgDiff) {
			line.insert(line.begin(), cv::Point2f(line.front().x, bounds.first));
#ifndef NDEBUG
			stats.boundary++;
#endif
		}
		if (std::abs(line.back().y - bounds.second) > 0.5*avgDiff) {
			line.push_back(cv::Point2f(line.back().x, bounds.second));
#ifndef NDEBUG
			stats.boundary++;
#endif
		}
	}
	// intermediate lines
	for (std::size_t i = 2; i < filteredList.size() - 2; ++i) {
		Line &line = *filteredList[i];
		double avgDiff = (i&1u) ? avgEven : avgOdd;
		std::pair<double, double> bounds = (i&1u) ? boundsEven : boundsOdd;
		if (std::abs(line.front().y - bounds.first) > 0.5*avgDiff) {
			line.insert(line.begin(), cv::Point2f(line.front().x, bounds.first));
#ifndef NDEBUG
			stats.boundary++;
#endif
		}
		if (std::abs(line.back().y - bounds.second) > 0.5*avgDiff) {
			line.push_back(cv::Point2f(line.back().x, bounds.second));
#ifndef NDEBUG
			stats.boundary++;
#endif
		}
	}
	// special handling of the last two lines
	for (std::size_t i = filteredList.size() - 2; i < filteredList.size(); ++i) {
		Line &line = *filteredList[i];
		double avgDiff = (i&1u) ? avgEven : avgOdd;
		std::pair<double, double> bounds = (i&1u) ? boundsEven : boundsOdd;
		if (std::abs(line.front().y - bounds.first) > 0.5*avgDiff) {
			line.insert(line.begin(), cv::Point2f(line.front().x, bounds.first));
#ifndef NDEBUG
			stats.boundary++;
#endif
		}
		if (std::abs(line.back().y - bounds.second) > 0.5*avgDiff) {
			line.push_back(cv::Point2f(line.back().x, bounds.second));
#ifndef NDEBUG
			stats.boundary++;
#endif
		}
	}

	// now go through all lines and detect and missing centroids, possibly also remove outliers
	// TODO

#ifndef NDEBUG
	std::cout << "removed " << stats.shortlines << " short lines" << std::endl;
	std::cout << "added " << stats.boundary << " boundary points" << std::endl;
#endif

	return filtered;
}

}
}
