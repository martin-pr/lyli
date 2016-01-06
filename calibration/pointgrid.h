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
#ifndef LYLI_CALIBRATION_POINTGRID_H_
#define LYLI_CALIBRATION_POINTGRID_H_

#include <functional>
#include <map>
#include <memory>
#include <opencv2/core/core.hpp>
#include <unordered_map>
#include <vector>

#include <calibration/subgrid.h>

namespace Lyli {
namespace Calibration {

/**
 * A grid of points representing the detected lens centroids joined to lines.
 *
 * The line grid has three step construction. First an empty grid is
 * constructed. The grid is then iteratively populated using points
 * representing lens centroids. Finally, the construction is finished
 * by calling the finalize() function
 *
 */
class PointGrid {
public:
	/// The limiting coordinate until which new lines may be constructed
	constexpr static int CONSTRUCT_LIM = 20;
	/// max difference in pixels for constructing new lines
	constexpr static float MAX_DIFF = 3.0;

	class Point;
	/// Line consisting of pointers to the the points (points are stored separately)
	struct Line {
		SubGrid subgrid;
		std::vector<Point*> line;
	};
	/// List of lines
	using LineList = std::vector<Line>;

	/**
	 * Point in PointGrid.
	 *
	 * The point has links to the lines containing the point (ie. lines whose intersection
	 * is the point).
	 *
	 * TODO: make the point movable as soon as OpenCV gets movable Point2f
	 */
	class Point {
	public:
		friend class PointGrid;

		~Point();
		Point(const Point &point);
		Point& operator=(const Point &point);

		const cv::Point2f& getPosition() const;
		std::size_t getHorizontalLineIndex() const;
		std::size_t getVerticalLineIndex() const;

	protected:
		/// The point is constructible only by PointGrid
		Point(const cv::Point2f &pos);

	private:
		cv::Point2f position;
		std::size_t horizontalLine;
		std::size_t verticalLine;
	};

	/**
	 * Default constructor.
	 */
	PointGrid();
	/**
	 * Copy constructor.
	 */
	PointGrid(const PointGrid &other);
	/**
	 * A destructor
	 */
	~PointGrid();
	/**
	 * Assignment operator.
	 */
	PointGrid &operator=(const PointGrid &other);

	/**
	 * Add a point to the grid.
	 *
	 * The points need to be added in increasing y-order.
	 */
	void addPoint(const cv::Point2f &point);

	/**
	 * Finalize the construction of line grid.
	 *
	 * The points whose y-dimension is in the CONSTRUCT_LIM range in the middle
	 * of the image are used to construct a new horizontal line if no suitable
	 * line was found. Otherwise the point is just added to a suitable closest line.
	 *
	 * The remaining points are either added to a suitable line or they are not
	 * added at all.
	 *
	 * Vertical lines are constructed in a similar fashion.
	 *
	 * The points that doesn't correspond to both horizontal and vertical line
	 * are removed
	 */
	void finalize();

	/**
	 * Test whether the grid contains any lines.
	 */
	bool isEmpty() const;

	/**
	 * Get the horizontal lines
	 */
	const LineList& getHorizontalLines() const;
	/**
	 * Get vertical lines.
	 */
	const LineList& getVerticalLines() const;

private:
	using PointStore = std::unordered_map<Point*, std::unique_ptr<Point>>;
	using TmpLineMap = std::map<float, Line>;
	using PointAccumulator = std::vector<Point*>;

	/**
	 * The accumulator is used to preserve the order of points as they are added
	 * to allow later construction in finalize()
	 */
	PointAccumulator accumulator;

	/**
	 * Point storage.
	 * The storage holds instances of points in unique_ptr. Additionaly, it stores
	 * the mappinng between RAW pointers to the held instance to allow quick lookup
	 * of instance. Unordered set would seem more suitable for this, but note that
	 * we can't effectively search unique_ptr in set.
	 */
	PointStore storage;
	/// Map of horizontal lines
	LineList linesHorizontal;
	/// Map of vertical lines
	LineList linesVertical;

	/**
	 * Add point to the storage.
	 *
	 * \return pointer to a point in storage
	 */
	Point* storageAdd(const cv::Point2f &point);
	/**
	 * Add pointer to the selected line map and construct a new line
	 * if necessary.
	 *
	 * \param lineMap map to add point to
	 * \param position position that serves as a key
	 * \param point to add
	 */
	void mapAddConstruct(TmpLineMap &lineMap, float position, Point *point);
	/**
	 * Add pointer to the selected line map if there is a suitable line
	 *
	 * \param lineMap map to add point to
	 * \param position position that serves as a key
	 * \param point to add
	 */
	void mapAdd(TmpLineMap &lineMap, float position, Point *point);
	/**
	 * Helper function to insert points to horizontal lines.
	 *
	 * \param start start of the line range to process (inclusive)
	 * \param end end of the line range (exclusive).
	 *        end may be smaller than start, in which case the loop is executed in reverse.
	 * \param inserter function that processes a point and adds it a corresponding line
	 */
	void horizontalLineInserter(int start, int end,
	                             std::function<void(Point *)> inserter);
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
	                             std::function<void(Point *)> inserterOdd,
	                             std::function<void(Point *)> inserterEven);
};

}
}

#endif
