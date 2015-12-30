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

namespace Lyli {
namespace Calibration {

class PointGrid;

/**
 * A grid of points representing the detected lens centroids joined to lines.
 *
 * Because the grid is in fact a hexagonal grid, every other row is offset.
 */
class LineGrid {
public:
	/// List of line positions
	using LineList = std::vector<float>;

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
	 * Get vertical lines for odd horizontal lines.
	 */
	const LineList& getVerticalLinesOdd() const;
	/**
	 * Get vertical lines for even horizontal lines.
	 */
	const LineList& getVerticalLinesEven() const;

private:
	LineList horizonalLines;
	LineList verticalOddLines;
	LineList verticalEvenLines;
};

}
}

#endif
