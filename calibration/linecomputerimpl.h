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
#ifndef LYLI_CALIBRATION_LINECOMPUTER_IMPL_H_
#define LYLI_CALIBRATION_LINECOMPUTER_IMPL_H_

#include "linecomputer.h"

#include <cmath>
#include <iterator>
#include <limits>

namespace Lyli {
namespace Calibration {

template <typename LineType>
LineType* LineComputer<LineType>::findHead(float position) {
	// initial fill - always create a new line
	if (lineMap.empty()) {
		auto res = lineMap.emplace(position, LineType());
		// return pointer to the new line
		return &(res.first->second);
	}

	auto ub = lineMap.lower_bound(position);
	auto lb = ub != lineMap.begin() ? std::prev(ub) : lineMap.end();

	float diffLb = lb != lineMap.end() ? std::abs(lb->first - position) : std::numeric_limits<float>::max();
	float diffUb = ub != lineMap.end() ? std::abs(ub->first - position) : std::numeric_limits<float>::max();

	auto lineIt = diffLb < diffUb ? lb : ub;

	// if a point is far from its bounds, it creates a new line
	if(std::abs(lineIt->first - position) > MAX_DIFF) {
		// construct a new line
		auto res = lineMap.emplace(position, LineType());
		lineIt = res.first; // TODO: error handling
	}
	else {
		// update the key
		auto line = lineIt ->second;
		auto nextLineIt = lineMap.erase(lineIt);
		lineIt = lineMap.insert(nextLineIt, std::make_pair(position, line));
	}

	// return pointer to the closest line
	return &(lineIt->second);
}

template <typename LineType>
LineType* LineComputer<LineType>::find(float position) {
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
		// return pointer to the line
		return &(lineIt->second);
	}
	else {
		return nullptr;
	}
}

template <typename LineType>
typename LineComputer<LineType>::LineMap LineComputer<LineType>::getLineMap() const {
	return lineMap;
}

}
}

#endif
