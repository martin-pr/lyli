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

#include <json/value.h>

#include "pointgrid.h"

namespace Lyli {
namespace Calibration {

LineGrid::Line::Line() : subgrid(SubGrid::SUBGRID_A), position(0.0f) {

}

LineGrid::Line::Line(SubGrid sub, float pos) : subgrid(sub), position(pos) {

}

Json::Value LineGrid::Line::serialize() const {
	Json::Value root(Json::objectValue);
	root["subgrid"] = static_cast<int>(subgrid);
	root["position"] = position;
	return root;
}

void LineGrid::Line::deserialize(const Json::Value& value) {
	subgrid = static_cast<SubGrid>(value["subgrid"].asInt());
	position = value["position"].asFloat();
}

LineGrid::LineGrid(const PointGrid &pointGrid) {
	// reserve space for more efficient operation
	horizonalLines.reserve(pointGrid.getHorizontalLines().size());
	verticalLines.reserve(pointGrid.getVerticalLines().size());

	// set the x value of each horizontal line in LineGrid to the average x of the points in PointGrid
	for (std::size_t i = 0; i < pointGrid.getHorizontalLines().size(); ++i) {
		const auto &line(pointGrid.getHorizontalLines()[i]);
		// first compute average
		double sum = 0.0;
		double count = 0.0;
		for (std::size_t i = line.line.size() / 3; i < 2 * line.line.size() / 3; ++i) {
			sum += line.line[i]->getPosition().x;
			count += 1.0;
		}
		// add a new line as the average of all points in PointGrid line
		horizonalLines.push_back(Line(line.subgrid, sum / count));
	}
	// set the y value in each vertical line for odd rows to the average
	for (std::size_t i = 0; i < pointGrid.getVerticalLines().size(); ++i) {
		const auto &line(pointGrid.getVerticalLines()[i]);
		// first compute average
		double sum = 0.0;
		double count = 0.0;
		std::size_t start = line.line.size() / 3;
		start = (start & 1) == 0 ? start : start - 1;
		for (std::size_t i = start; i < 2 * line.line.size() / 3; i += 2) {
			sum += line.line[i]->getPosition().y;
			count += 1.0;
		}
		// add a new line as the average of all points in PointGrid line
		verticalLines.push_back(Line(line.subgrid, sum / count));
	}
}

LineGrid::LineGrid() {

}

LineGrid::~LineGrid() {

}

LineGrid::LineGrid(const LineGrid &other) : horizonalLines(other.horizonalLines), verticalLines(other.verticalLines) {

}

LineGrid &LineGrid::operator=(const LineGrid &other) {
	if(this != &other) {
		LineGrid tmp(other);
		std::swap(horizonalLines, tmp.horizonalLines);
		std::swap(verticalLines, tmp.verticalLines);
	}
	return *this;
}

const LineGrid::LineList& LineGrid::getHorizontalLines() const {
	return horizonalLines;
}

const LineGrid::LineList& LineGrid::getVerticalLines() const {
	return verticalLines;
}

Json::Value LineGrid::serialize() const {
	Json::Value root(Json::objectValue);
	root["horizontal"] = Json::Value(Json::arrayValue);
	for (std::size_t i = 0; i < horizonalLines.size(); ++i) {
		root["horizontal"][static_cast<int>(i)] = horizonalLines[i].serialize();
	}
	root["vertical"] = Json::Value(Json::arrayValue);
	for (std::size_t i = 0; i < verticalLines.size(); ++i) {
		root["vertical"][static_cast<int>(i)] = verticalLines[i].serialize();
	}
	return root;
}

void LineGrid::deserialize(const Json::Value& value) {
	const Json::Value& horizontal = value["horizontal"];
	for (Json::Value::ArrayIndex i = 0, end = horizontal.size(); i < end; ++i) {
		Line line;
		line.deserialize(horizontal[i]);
		horizonalLines.push_back(line);
	}
	const Json::Value& vertical = value["vertical"];
	for (Json::Value::ArrayIndex i = 0, end = vertical.size(); i < end; ++i) {
		Line line;
		line.deserialize(vertical[i]);
		verticalLines.push_back(line);
	}
}


}
}
