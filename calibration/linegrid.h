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

#include "line.h"
#include "linemap.h"

#include <functional>
#include <map>
#include <memory>
#include <opencv2/core/core.hpp>
#include <unordered_map>
#include <vector>

namespace Lyli {
namespace Calibration {

/**
 * A grid of lines constructed from the detected lens centroids.
 *
 * The line grid has three step construction. First an empty grid is
 * constructed. The grid is then iteratively populated using points
 * representing lens centroids. Finally, the construction is finished
 * by calling the finalize() function
 *
 */
class LineGrid {
public:
	/// The limiting coordinate until which new lines may be constructed
	constexpr static int CONSTRUCT_LIM = 20;
	/// max difference in pixels for constructing new lines
	constexpr static float MAX_DIFF = 3.0;

	/**
	 * Add a point to the grid.
	 *
	 * The points whose y-dimension is smaller than CONSTRUCT_LIM will
	 * construct a new line if no suitable line was found. Otherwise the
	 * point is just added to a suitable closest line.
	 *
	 * The remaining points (y > CONSTRUCT_LIM) are either added to a suitable
	 * line or they are not added at all.
	 */
	void addPoint(const cv::Point2f &point);

	/**
	 * Finalize the construction of line grid.
	 */
	void finalize();

	/**
	 * Get the map storing horizontal lines
	 */
	const LineMap& getHorizontalMap() const;
	/**
	 * Get the map storing odd vertical lines
	 */
	const LineMap& getVerticalMapOdd() const;
	/**
	 * Get the map storing even vertical lines
	 */
	const LineMap& getVerticalMapEven() const;

private:
	using PointMap = std::unordered_map<cv::Point2f*, std::unique_ptr<cv::Point2f>>;
	using TmpLineMap = std::map<float, PtrLine>;

	// temporary line map
	TmpLineMap tmpLineMap;

	/// The point storage
	PointMap storage;
	/// Map of horizontal lines
	LineMap lineMapHorizontal;
	/// Map of odd vertical lines
	LineMap lineMapVerticalOdd;
	/// Map of even vertical lines
	LineMap lineMapVerticalEven;

	/**
	 * Add point to the storage.
	 *
	 * \return pointer to a point in storage
	 */
	cv::Point2f* storageAdd(const cv::Point2f &point);
	/**
	 * Add pointer to the selected line map and construct a new line
	 * if necessary.
	 *
	 * \param lineMap map to add point to
	 * \param position position that serves as a key
	 * \param point to add
	 */
	void mapAddConstruct(TmpLineMap &lineMap, float position, cv::Point2f *point);
	/**
	 * Add pointer to the selected line map if there is a suitable line
	 *
	 * \param lineMap map to add point to
	 * \param position position that serves as a key
	 * \param point to add
	 */
	void mapAdd(TmpLineMap &lineMap, float position, cv::Point2f *point);
	/**
	 * Helper function to construct vertical lines
	 *
	 * \param start start of the line range to process (inclusive)
	 * \param end end of the line range (exclusive).
	 *        end may be smaller than start, in which case the loop is executed in reverse.
	 * \param inserterOdd function that processes a point and adds it a corresponding line, used for odd lines
	 * \param inserterEven as above, used for even lines
	 */
	void verticalLineConstructor(int start, int end,
	                             std::function<void(cv::Point2f *)> inserterOdd,
	                             std::function<void(cv::Point2f *)> inserterEven);
};

}
}

#endif
