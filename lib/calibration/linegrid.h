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
#ifndef LYLI_CALIBRATION_LINEGRID_H_
#define LYLI_CALIBRATION_LINEGRID_H_

#include <utility>
#include <vector>

#include <calibration/subgrid.h>

namespace Json {
class Value;
}

namespace Lyli {
namespace Calibration {

class GridMapper;
class PointGrid;

/**
 * A grid of points representing the detected lens centroids joined to lines.
 *
 * Because the grid is in fact a hexagonal grid, every other row is offset.
 */
class LineGrid {
public:
	struct Line {
		Line();
		Line(SubGrid subgrid, float position);

		/**
		 * Serialize into a JSON object
		 * \return JSON object representing the class
		 */
		Json::Value serialize() const;
		/**
		 * Deserialize from a JSON object
		 * \param value JSON object representing the class
		 */
		void deserialize(const Json::Value& value);

		SubGrid subgrid;
		float position;
	};
	/// List of line positions
	using LineList = std::vector<Line>;

	friend std::pair<LineGrid, std::vector<GridMapper>> averageGrids(const std::vector<LineGrid> &grids);

	/**
	 * Construct LineGrid from PointGrid.
	 */
	explicit LineGrid(const PointGrid &pointGrid);

	/**
	 * Default constructor.
	 */
	LineGrid();
	/**
	 * A destructor
	 */
	~LineGrid();
	/**
	 * Copy constructor.
	 */
	LineGrid(const LineGrid &other);
	/**
	 * Assignment operator.
	 */
	LineGrid &operator=(const LineGrid &other);

	/**
	 * Get the horizontal lines
	 */
	const LineList& getHorizontalLines() const;
	/**
	 * Get the vertical lines.
	 */
	const LineList& getVerticalLines() const;

	/**
	 * Serialize into a JSON object
	 * \return JSON object representing the class
	 */
	Json::Value serialize() const;
	/**
	 * Deserialize from a JSON object
	 * \param value JSON object representing the class
	 */
	void deserialize(const Json::Value& value);

private:
	LineList horizonalLines;
	LineList verticalLines;
};

}
}

#endif
