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

#ifndef LYLI_CALIBRATION_LINECOMPUTER_H_
#define LYLI_CALIBRATION_LINECOMPUTER_H_

#include <map>

namespace Lyli {
namespace Calibration {

/**
 * A class providing all necessary functions to create lines from separate points.
 *
 * \tparam LineType the type for line storage
 */
template <typename LineType>
class LineComputer {
public:
	typedef std::map<float, LineType> LineMap; //!< maps the y-position of the last centroid for each line with the line

	/**
	 * Find a line for position which may create a new line.
	 *
	 * The findHead() function should be used on a selected first number of columns to obtain
	 * the beginning of all lines in the image. In contrary to add(), this function
	 * can create new line entries in the map.
	 *
	 * \param position position to detect line
	 */
	LineType* findHead(float position);

	/**
	 * Find a line for given position.
	 *
	 * The function tries to find the line to which the centroid corresponds. The points too
	 * far off are ignored, as these are likely noise rather than lens centroids.
	 *
	 * \param position position to detect line
	 * \return pointer to the line or nullptr is the position is too far from any line
	 */
	LineType* find(float position);

	LineMap getLineMap() const;

private:
	constexpr static float MAX_DIFF = 3.0; //!< max difference in pixels

	LineMap lineMap;
};

}
}

#endif
