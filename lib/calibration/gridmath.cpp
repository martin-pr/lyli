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
#include "gridmath.h"

#include <cmath>
#include <limits>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

constexpr float LIMIT_HORIZONTAL = 17.0f;
constexpr float LIMIT_VERTICAL = 8.0f;

struct LineEntry {
	/// pair <grid index, line index>
	using Referee = std::pair<std::size_t, std::size_t>;

	LineEntry() : counter(0) {

	}

	LineEntry(const Lyli::Calibration::LineGrid::Line &line_, const Referee &referee) : line(line_), counter(1) {
		referees.push_back(referee);
	}

	Lyli::Calibration::LineGrid::Line line; // line
	unsigned int counter; // count from how many grids the line was  constructed
	// stores the pair <grid index, line index> of the lines that were used in computation of this line
	std::vector<Referee> referees;
};

using LineMap = std::map<float, LineEntry>;

/**
 * Insert a new line at the correct position in a map
 */
void insertLine(LineMap &map, const Lyli::Calibration::LineGrid::Line &line, const LineEntry::Referee &referee, float limit) {
	auto ub = map.lower_bound(line.position);
	auto lb = ub != map.begin() ? std::prev(ub) : map.end();

	float diffLb = lb != map.end() ? std::abs(lb->first - line.position) : std::numeric_limits<float>::max();
	float diffUb = ub != map.end() ? std::abs(ub->first - line.position) : std::numeric_limits<float>::max();

	auto lineIt = diffLb < diffUb ? lb : ub;

	if (std::abs(lineIt->first - line.position) < limit) {
		// replace the line entry with an average
		LineEntry entry(std::move(lineIt->second));
		entry.referees.push_back(referee);
		double denom = 1.0 / (entry.counter + 1.0);
		entry.line.position = entry.counter*denom*entry.line.position + line.position*denom;
		++entry.counter;
		auto nextEntryIt = map.erase(lineIt);
		lineIt = map.emplace_hint(nextEntryIt, entry.line.position, std::move(entry));
	}
	else {
		map.emplace(line.position, LineEntry(line, referee));
	}
}

}

namespace Lyli {
namespace Calibration {

std::pair<LineGrid, std::vector<GridMapper>> averageGrids(const std::vector<LineGrid> &grids) {
	// maps storing the lines of the resulting average map
	LineMap horizontal;
	LineMap vertical;

	// initialize the map for average line grid with the first line grid
	std::size_t lineIndex = 0;
	for (const auto &line : grids.front().getHorizontalLines()) {
		horizontal.emplace(line.position, LineEntry(line, std::make_pair(0, lineIndex++)));
	}
	lineIndex = 0;
	for (const auto &line : grids.front().getVerticalLines()) {
		vertical.emplace(line.position, LineEntry(line, std::make_pair(0, lineIndex++)));
	}

	// now add lines from the remaining grids
	std::size_t gridIndex = 1;
	for (auto it(grids.begin() + 1); it != grids.end(); ++it) {
		lineIndex = 0;
		for (const auto &line : it->getHorizontalLines()) {
			insertLine(horizontal, line, std::make_pair(gridIndex, lineIndex++), LIMIT_HORIZONTAL);
		}
		lineIndex = 0;
		for (const auto &line : it->getVerticalLines()) {
			insertLine(vertical, line, std::make_pair(gridIndex, lineIndex++), LIMIT_VERTICAL);
		}
		++gridIndex;
	}

	// prepare grid mappers
	std::vector<GridMapper> mappers;
	mappers.reserve(grids.size());
	for (const auto &grid : grids) {
		mappers.push_back(GridMapper(grid.getHorizontalLines().size(), grid.getVerticalLines().size()));
	}

	// construct the line grid and store the mapping for each line
	LineGrid lineGrid;
	lineIndex = 0;
	for (const auto& line : horizontal) {
		if (line.second.counter > grids.size() / 2) {
			lineGrid.horizonalLines.push_back(line.second.line);
			// store mapping
			for (const auto &referee : line.second.referees) {
				mappers[referee.first].mapHorizontal(referee.second, lineIndex);
			}
			++lineIndex;
		}
	}
	lineIndex = 0;
	for (const auto& line : vertical) {
		if (line.second.counter > grids.size() / 2) {
			lineGrid.verticalLines.push_back(line.second.line);
			// store mapping
			for (const auto &referee : line.second.referees) {
				mappers[referee.first].mapVertical(referee.second, lineIndex);
			}
			++lineIndex;
		}
	}

	return std::make_pair(lineGrid, mappers);
}

}
}
