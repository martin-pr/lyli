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

#include "pointgrid.h"

namespace Lyli {
namespace Calibration {

LineGrid::LineGrid(const PointGrid &pointGrid) {
	LineGrid lineGrid;

	// reserve space for more efficient operation
	lineGrid.horizonalLines.reserve(pointGrid.getHorizontalLines().size());
	lineGrid.verticalOddLines.reserve(pointGrid.getVerticalLinesOdd().size());
	lineGrid.verticalEvenLines.reserve(pointGrid.getVerticalLinesEven().size());

	// set the x value of each horizontal line in LineGrid to the average x of the points in PointGrid
	for (std::size_t i = 0; i < pointGrid.getHorizontalLines().size(); ++i) {
		const auto &line(pointGrid.getHorizontalLines()[i]);
		// first compute average
		double sum = 0.0;
		double count = 0.0;
		for (std::size_t i = line.size() / 3; i < 2 * line.size() / 3; ++i) {
			sum += line[i]->getPosition().x;
			count += 1.0;
		}
		// add a new line as the average of all points in PointGrid line
		lineGrid.horizonalLines.push_back(sum / count);
	}
	// set the y value in each vertical line for odd rows to the average
	for (std::size_t i = 0; i < pointGrid.getVerticalLinesOdd().size(); ++i) {
		const auto &line(pointGrid.getVerticalLinesOdd()[i]);
		// first compute average
		double sum = 0.0;
		double count = 0.0;
		std::size_t start = line.size() / 3;
		start = (start & 1) == 0 ? start : start - 1;
		for (std::size_t i = start; i < 2 * line.size() / 3; i += 2) {
			sum += line[i]->getPosition().y;
			count += 1.0;
		}
		// add a new line as the average of all points in PointGrid line
		lineGrid.verticalOddLines.push_back(sum / count);
	}
	// set the y value in each vertical line for even rows to the average
	for (std::size_t i = 0; i < pointGrid.getVerticalLinesEven().size(); ++i) {
		const auto &line(pointGrid.getVerticalLinesEven()[i]);
		// first compute average
		double sum = 0.0;
		double count = 0.0;
		std::size_t start = line.size() / 3;
		start = (start & 1) != 0 ? start : start - 1;
		for (std::size_t i = start; i < 2 * line.size() / 3; i += 2) {
			sum += line[i]->getPosition().y;
			count += 1.0;
		}
		// add a new line as the average of all points in PointGrid line
		lineGrid.verticalEvenLines.push_back(sum / count);
	}
}

LineGrid::LineGrid() {

}

LineGrid::~LineGrid() {

}

LineGrid::LineGrid(const LineGrid &other) :
	horizonalLines(other.horizonalLines), verticalOddLines(other.verticalOddLines), verticalEvenLines(other.verticalEvenLines) {

}

LineGrid &LineGrid::operator=(const LineGrid &other) {
	if(this != &other) {
		LineGrid tmp(other);
		std::swap(horizonalLines, tmp.horizonalLines);
		std::swap(verticalOddLines, tmp.verticalOddLines);
		std::swap(verticalEvenLines, tmp.verticalEvenLines);
	}
	return *this;
}

const LineGrid::LineList& LineGrid::getHorizontalLines() const {
	return horizonalLines;
}

const LineGrid::LineList& LineGrid::getVerticalLinesOdd() const {
	return verticalOddLines;
}

const LineGrid::LineList& LineGrid::getVerticalLinesEven() const {
	return verticalEvenLines;
}

}
}
