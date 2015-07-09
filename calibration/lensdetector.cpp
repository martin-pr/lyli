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

#include "lensdetector.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace {

/**
 * Finds the centroid of an object in image starting at the position start
 * while filling the mask
 */
cv::Point2f findCentroid(const cv::Mat &image, cv::Mat &mask, cv::Point2i start) {
	std::uint8_t *data = reinterpret_cast<std::uint8_t*>(image.data);
	std::uint8_t *maskData = reinterpret_cast<std::uint8_t*>(mask.data);
	// we discover points based on a modified non-recursive flood fill algorithm that works
	// on monotone polygons only
	/*
	 * The algorithm works as follows:
	 *   1. Initialization: a startx position is set to the x position of the topmost
	 *      pixel of the object, endx to startx + 1.
	 *   2. Fill the current row.
	 *   3. Move to the startx position in the next row.
	 *   4. If the position is inside the object, start filling both sides.
	 *      If the position is not inside the object, the object is searched in the
	 *      increasing x-direction.
	 *   4. A new startx position is set to the lowest x-coordinate of the filled row.
	 *      The endx value is updated to the rightmost pixel.
	 *   5. goto 3
	 *
	 *   6. Stop condition: no white pixel is found before reaching endx.
	 */

	// skip the objects one pixel from the edge
	if (start.x == image.cols - 1) {
		return cv::Point2f(0.0, 0.0);
	}

	int startx = start.x;
	int endx = start.x + 1;
	int y = start.y;
	double m01 = 0.0;
	double m10 = 0.0;
	double sum = 0.0;

	while (y < image.rows) {
		int pos = image.cols * y + startx;
		int endpos = image.cols * y + endx;

		if (maskData[pos] == Lyli::Calibration::Mask::OBJECT) {
			// fill to the left
			int oldstartx = startx;
			int tmppos = pos - 1;
			int x = startx - 1;
			while(x >= 0 && maskData[tmppos] == Lyli::Calibration::Mask::OBJECT) {
				// compute
				m10 += y * data[tmppos];
				m01 += x * data[tmppos];
				sum += data[tmppos];
				maskData[tmppos] = Lyli::Calibration::Mask::PROCESSED;
				// move to next
				--tmppos;
				--startx;
				--x;
			}
			// fill to the right
			tmppos = pos;
			x = oldstartx;
			while(x < image.cols && maskData[tmppos] == Lyli::Calibration::Mask::OBJECT) {
				// compute
				m10 += y * data[tmppos];
				m01 += x * data[tmppos];
				sum += data[tmppos];
				maskData[tmppos] = Lyli::Calibration::Mask::PROCESSED;
				// move to next
				++tmppos;
				++x;
			}
			endx = x - 1;
		}
		else {
			// find the start position
			int tmppos = pos;
			while(maskData[tmppos] == Lyli::Calibration::Mask::EMPTY) {
				if (tmppos == endpos) {
					// stop fill
					goto findCentroid_stop;
				}
				// skip to next
				++tmppos;
				++startx;
			}
			// fill to the right
			int x = startx;
			while(x < image.cols && maskData[tmppos] == Lyli::Calibration::Mask::OBJECT) {
				// compute
				m10 += y * data[tmppos];
				m01 += x * data[tmppos];
				sum += data[tmppos];
				maskData[tmppos] = Lyli::Calibration::Mask::PROCESSED;
				// move to next
				++tmppos;
				++x;
			}
			endx = x - 1;
		}
		// move to the next row
		++y;
	}
	findCentroid_stop:

	return cv::Point2f(m01/sum, m10/sum);
}

/**
 * Get interpolated color at a non-integer position.
 *
 * Uses just bilinear interpolation.
 */
float getInterpolatedColor(const cv::Mat &image, cv::Point2f position) {
	assert(image.channels() == 1);

	const unsigned int xx = std::floor(position.x);
	const unsigned int yy = std::floor(position.y);
	int x0 = cv::borderInterpolate(xx,   image.cols, cv::BORDER_REFLECT_101);
	int x1 = cv::borderInterpolate(xx+1, image.cols, cv::BORDER_REFLECT_101);
	int y0 = cv::borderInterpolate(yy,   image.rows, cv::BORDER_REFLECT_101);
	int y1 = cv::borderInterpolate(yy+1, image.rows, cv::BORDER_REFLECT_101);

	const float f00 = image.at<uchar>(y0, x0);
	const float f01 = image.at<uchar>(y0, x1);
	const float f10 = image.at<uchar>(y1, x0);
	const float f11 = image.at<uchar>(y1, x1);

	const float x0dif = position.x - x0;
	const float x1dif = 1.0 - x0dif;
	const float y0dif = position.y - y0;
	const float y1dif = 1.0 - y0dif;

	// the denominator is always 1 (because we use x, x+1)
	return f00*y1dif*x1dif + f10*x0dif*y1dif + f01*x1dif*y0dif + f11*x0dif*y0dif;
}

std::vector<cv::Point2f> computeMask(int radius) {
	std::vector<cv::Point2f> mask;
	float r2 = radius*radius;

	for (int y = radius; y >= -radius; --y) {
		int x0 = std::round(std::sqrt(r2 - y*y));
		for (int x = -x0; x <= x0; ++x) {
			mask.push_back(cv::Point2f(x, y));
		}
	}

	mask.shrink_to_fit();
	return mask;
}

/**
 * Refine centroid.
 *
 * The initial centroid is iteratively refined using increasingly large circular neigborhood
 * to better estimate of centroid.
 */
cv::Point2f refineCentroid(const cv::Mat &image, cv::Point2i start) {
	// begin refining with radius 3px, stop at 5px radius
	// use precomputed masks with relative offset to the start
	static const std::vector<cv::Point2f> offsets3px = computeMask(3);
	static const std::vector<cv::Point2f> offsets4px = computeMask(4);
	static const std::vector<cv::Point2f> offsets5px = computeMask(5);
	static const std::vector<cv::Point2f> offsets6px = computeMask(6);

	double m01, m10, sum;
	cv::Point2f estimate(start.x, start.y);
	for (auto mask : { offsets3px, offsets4px, offsets5px, offsets6px } ) {
		m01 = 0.0;
		m10 = 0.0;
		sum = 0.0;
		for (auto point : mask) {
			auto pos = point + estimate;
			float pixel = getInterpolatedColor(image, pos);
			m10 += pos.y * pixel;
			m01 += pos.x * pixel;
			sum += pixel;
		}
		estimate = cv::Point2f(m01/sum, m10/sum);
	}

	return estimate;
}

/**
 * A class providing all necessary functions to create lines from separate points.
 */
class LineComputer {
public:
	typedef std::vector<cv::Point2f> Line; //!< a line of centroids
	typedef std::map<float, Line> LineMap; //!< maps the y-position of the last centroid for each line with the line

	/**
	 * Add a centroid that may start a new line.
	 *
	 * The addHead() function should be used on a selected first number of columns to obtain
	 * the beginning of all lines in the image. In contrary to add(), this function
	 * can create new line entries in the map.
	 */
	void addHead(float key, cv::Point2f point) {
		// initial fill - avoid need to check both lower and upper bound
		if (lineMap.size() < 2) {
			auto res = lineMap.emplace(key, Line());
			res.first->second.push_back(point);
			return;
		}

		auto ub = lineMap.lower_bound(key);
		auto lb = ub != lineMap.begin() ? std::prev(ub) : lineMap.end();

		float diffLb = lb != lineMap.end() ? std::abs(lb->first - key) : std::numeric_limits<float>::max();
		float diffUb = ub != lineMap.end() ? std::abs(ub->first - key) : std::numeric_limits<float>::max();

		auto lineIt = diffLb < diffUb ? lb : ub;

		// if the points is far from its bounds, it creates a new line
		if(std::abs(lineIt->first - key) > MAX_DIFF) {
			// construct a new line
			auto res = lineMap.emplace(key, Line());
			lineIt = res.first; // TODO: error handling
		}
		else {
			// update the key
			auto line = lineIt ->second;
			auto nextLineIt = lineMap.erase(lineIt);
			lineIt = lineMap.insert(nextLineIt, std::make_pair(key, line));
		}

		// add point to the line
		lineIt->second.push_back(point);
	}

	/**
	 * Add a new centroid to the computer.
	 *
	 * The function tries to find the line to which the centroid corresponds. The points too
	 * far off are ignored, as these are likely noise rather than lens centroids.
	 */
	void add(float key, cv::Point2f point) {
		auto ub = lineMap.lower_bound(key);
		auto lb = ub != lineMap.begin() ? std::prev(ub) : lineMap.end();

		float diffLb = lb != lineMap.end() ? std::abs(lb->first - key) : std::numeric_limits<float>::max();
		float diffUb = ub != lineMap.end() ? std::abs(ub->first - key) : std::numeric_limits<float>::max();

		auto lineIt = diffLb < diffUb ? lb : ub;

		// if the points is far from its bounds, it creates a new line
		if(std::abs(lineIt->first - key) < MAX_DIFF) {
			// update the key
			auto line = lineIt ->second;
			auto nextLineIt = lineMap.erase(lineIt);
			lineIt = lineMap.insert(nextLineIt, std::make_pair(key, line));
			// add point to the line
			lineIt->second.push_back(point);
		}
	}

	LineMap getLineMap() const {
		return lineMap;
	}

private:
	constexpr static float MAX_DIFF = 3.0; //!< max difference in pixels

	LineMap lineMap;
};

}

namespace Lyli {
namespace Calibration {

LineMap LensDetector::detect(const cv::Mat& gray, cv::Mat& mask) {
	constexpr static int HEADER = 20;

	// transpose the greyMat image, as its easier to scan row by row rather than by column
	// while row scanning on original is not problem for finding centroids, line detection
	// needs to sweep orthogonaly to lines
	cv::Mat greyMatTranspose(gray.t());
	cv::Mat maskTranspose(mask.t());

	LineComputer lineComp;
	// find centroids and create map of lines
	for (int row = 0; row < HEADER; ++row) {
		std::uint8_t* pixel = maskTranspose.ptr<std::uint8_t>(row);
		for (int col = 0; col < maskTranspose.cols; ++col) {
			// if the pixel is not black and it is not marked, process
			if (pixel[col] == Mask::OBJECT) {
				cv::Point2f centroid = findCentroid(greyMatTranspose, maskTranspose, cv::Point2i(col, row));
				centroid = refineCentroid(greyMatTranspose, centroid);
				// construct line map
				lineComp.addHead(centroid.x, centroid);

				// DEBUG: store centroid in image
				maskTranspose.at<uchar>(centroid) = 192;
			}
		}
	}

	for (int row = HEADER; row < maskTranspose.rows; ++row) {
		std::uint8_t* pixel = maskTranspose.ptr<std::uint8_t>(row);
		// find centroids and complete the lines by adding remaining centroids
		for (int col = 0; col < maskTranspose.cols; ++col) {
			// if the pixel is not black and it is not marked, process
			if (pixel[col] == Mask::OBJECT) {
				cv::Point2f centroid = findCentroid(greyMatTranspose, maskTranspose, cv::Point2i(col, row));
				centroid = refineCentroid(greyMatTranspose, centroid);
				// complete the line
				lineComp.add(centroid.x, centroid);

				// DEBUG: store centroid in image
				maskTranspose.at<uchar>(centroid) = 192;
			}
		}
	}

	// DEBUG: draw lines
	LineComputer::LineMap lineMap = lineComp.getLineMap();
	maskTranspose = cv::Scalar(256, 256, 256);
	for (auto line : lineMap) {
		for (std::size_t i = 1; i < line.second.size(); ++i) {
			cv::line(maskTranspose, line.second.at(i-1), line.second.at(i), cv::Scalar(0, 0, 0));
		}
	}

	// DEBUG: transpose the debug image
	mask = maskTranspose.t();

	return lineMap;
}

}
}
