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

#include "pointgrid.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace {

/**
 * A constant that limits the find centroid search to search MAX_LENS_SIZE pixels
 * from the start point at most.
 */
constexpr int MAX_LENS_SIZE = 15;

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

	// limits for search
	const int maxy = std::min(y + MAX_LENS_SIZE, image.rows);
	const int maxx = std::min(startx + MAX_LENS_SIZE, image.cols);
	const int minx = std::max(startx - MAX_LENS_SIZE, 0);

	// the search algorithm
	while (y < maxy) {
		int pos = image.cols * y + startx;
		int endpos = image.cols * y + endx;

		if (maskData[pos] == Lyli::Calibration::Mask::OBJECT) {
			// fill to the left
			int oldstartx = startx;
			int tmppos = pos - 1;
			int x = startx - 1;
			while(x >= minx && maskData[tmppos] == Lyli::Calibration::Mask::OBJECT) {
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
			while(x < maxx && maskData[tmppos] == Lyli::Calibration::Mask::OBJECT) {
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
			// compare against OBJECT rather than EMPTY, as we may hit PROCESSED pixels too
			// in case there is a little "spur" that sticks out on top of already processed pixels
			// which may happen if there are lenses that are fused together in the image
			while(maskData[tmppos] != Lyli::Calibration::Mask::OBJECT) {
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
			while(x < maxx && maskData[tmppos] == Lyli::Calibration::Mask::OBJECT) {
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

	// return 0 if the position is out of bounds
	if (position.x < 0 || position.x > image.cols - 1 || position.y < 0 || position.y > image.rows - 1) {
		return 0.0f;
	}

	const unsigned int xx = std::floor(position.x);
	const unsigned int yy = std::floor(position.y);
	int x0 = xx;
	int x1 = xx + 1;
	int y0 = yy;
	int y1 = yy + 1;

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
cv::Point2f refineCentroid(const cv::Mat &image, cv::Point2f start) {
	// begin refining with radius 3px, stop at 5px radius
	// use precomputed masks with relative offset to the start
	static const std::vector<cv::Point2f> offsets3px = computeMask(3);
	static const std::vector<cv::Point2f> offsets4px = computeMask(4);
	static const std::vector<cv::Point2f> offsets5px = computeMask(5);
	static const std::vector<cv::Point2f> offsets6px = computeMask(6);
	static const auto offsetlist = { offsets3px, offsets4px, offsets5px, offsets6px };

	double m01, m10, sum;
	cv::Point2f estimate(start.x, start.y);
	for (const auto &mask : offsetlist) {
		m01 = 0.0;
		m10 = 0.0;
		sum = 0.0;
		for (const auto &point : mask) {
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

}

namespace Lyli {
namespace Calibration {

PointGrid LensDetector::detect(const cv::Mat& gray, cv::Mat& mask) {
	// transpose the greyMat image, as its easier to scan row by row rather than by column
	// while row scanning on original is not problem for finding centroids, line detection
	// needs to sweep orthogonaly to lines
	cv::Mat greyMatTranspose(gray.t());
	cv::Mat maskTranspose(mask.t());

	PointGrid pointGrid;
	// find centroids and create map of lines
	for (int row = 0; row < maskTranspose.rows; ++row) {
		std::uint8_t* pixel = maskTranspose.ptr<std::uint8_t>(row);
		for (int col = 0; col < maskTranspose.cols; ++col) {
			// if the pixel is not black and it is not marked, process
			if (pixel[col] == Mask::OBJECT) {
				cv::Point2f centroid = findCentroid(greyMatTranspose, maskTranspose, cv::Point2i(col, row));
				centroid = refineCentroid(greyMatTranspose, centroid);
				// construct line map
				pointGrid.addPoint(centroid);

				// DEBUG: store centroid in image
				maskTranspose.at<uchar>(centroid) = 192;
			}
		}
	}

	pointGrid.finalize();
	return pointGrid;
}

}
}
