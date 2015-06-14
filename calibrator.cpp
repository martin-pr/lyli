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

#include "calibrator.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

namespace {

constexpr std::uint8_t MASK_EMPTY = 0;
constexpr std::uint8_t MASK_PROCESSED = 128;
constexpr std::uint8_t MASK_OBJECT = 255;

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

		if (maskData[pos] == MASK_OBJECT) {
			// fill to the left
			int oldstartx = startx;
			int tmppos = pos - 1;
			int x = startx - 1;
			while(x >= 0 && maskData[tmppos] == MASK_OBJECT) {
				// compute
				m10 += y * data[tmppos];
				m01 += x * data[tmppos];
				sum += data[tmppos];
				maskData[tmppos] = MASK_PROCESSED;
				// move to next
				--tmppos;
				--startx;
				--x;
			}
			// fill to the right
			tmppos = pos;
			x = oldstartx;
			while(x < image.cols && maskData[tmppos] == MASK_OBJECT) {
				// compute
				m10 += y * data[tmppos];
				m01 += x * data[tmppos];
				sum += data[tmppos];
				maskData[tmppos] = MASK_PROCESSED;
				// move to next
				++tmppos;
				++x;
			}
			endx = x - 1;
		}
		else {
			// find the start position
			int tmppos = pos;
			while(maskData[tmppos] == MASK_EMPTY) {
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
			while(x < image.cols && maskData[tmppos] == MASK_OBJECT) {
				// compute
				m10 += y * data[tmppos];
				m01 += x * data[tmppos];
				sum += data[tmppos];
				maskData[tmppos] = MASK_PROCESSED;
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

// A class providing all necessary funcitons to create a line of points
// see also:
//   https://en.wikipedia.org/wiki/Simple_linear_regression
//   http://stats.stackexchange.com/questions/23481/are-there-algorithms-for-computing-running-linear-or-logistic-regression-param
//   http://stats.stackexchange.com/questions/22718/what-is-the-difference-between-linear-regression-on-y-with-x-and-x-with-y/22721#22721
class LineComputer {
public:
	typedef std::vector<cv::Point2f> Line;

	LineComputer() : n(0.0),
		x_sum(0.0), x2_sum(0.0),
		y_sum(0.0), y2_sum(0.0),
		xy_sum(0.0),
		line(std::make_shared<Line>()){

		line->reserve(1024);
	}

	/**
	 * Get the expected value of y for the given x
	 */
	double expect(double x) {
		if (n < 2.0) {
			return line->at(0).y;
		}

		double a = covariance() * (standardDeviationY() / standardDeviationX());
		double b = (y_sum / n) - a * (x_sum / n);
		return a*x + b;
	}

	/**
	 * Add point to the line
	 */
	void addPoint(const cv::Point2f point) {
		line->push_back(point);

		n += 1.0;
		x_sum += point.x;
		x2_sum += point.x * point.x;
		y_sum += point.y;
		y2_sum += point.y * point.y;
		xy_sum += point.x * point.y;
	}

	/**
	 * Sort the stored line by x
	 */
	void sortLine() {
		std::sort(line->begin(), line->end(), [](const cv::Point2f &a, const cv::Point2f &b) { return a.x < b.x; } );
	}

	/**
	 * Get the stored line
	 */
	std::shared_ptr<Line> getLine() {
		return line;
	}

private:
	double n; // number of the stored values
	double x_sum; //  sum of x
	double x2_sum; // sum of x^2
	double y_sum; // sum for the E[Y]
	double y2_sum; // sum for the E[Y^2]
	double xy_sum;

	std::shared_ptr<Line> line;

	double covariance() {
		return (n*xy_sum - x_sum*y_sum) / (std::sqrt(n*x2_sum - x_sum*x_sum) * std::sqrt(n*y2_sum - y_sum*y_sum));
	}

	double standardDeviationX() {
		double ex = x_sum / n; // expected value E[X]
		double ex2 = x2_sum / n; // // expected value E[X^2]
		return std::sqrt(ex2 - ex*ex);
	}

	double standardDeviationY() {
		double ey = y_sum / n;
		double ey2 = y2_sum / n;
		return std::sqrt(ey2 - ey*ey);
	}
};


}

namespace Lyli {

class Calibrator::Impl {
public:
	cv::Mat image;
	cv::Mat calibrationImage;
};

Calibrator::Calibrator() : pimpl(new Impl) {

}

Calibrator::~Calibrator() {

}

void Calibrator::addImage(const cv::Mat& image) {
	pimpl->image = image;
}

void Calibrator::calibrate() {
	cv::Mat dst, greyMat, tmp;

	// convert to gray
	cv::cvtColor(pimpl->image, tmp, CV_RGB2GRAY);
	tmp.convertTo(greyMat, CV_8U, 1.0/256.0);

	// find edges and apply threshold
	cv::Laplacian(greyMat, dst, CV_8U, 3);
	cv::threshold(dst, tmp, 64, 255, cv::THRESH_BINARY);

	// filter out small specks
	float m[3][3] = { { 0.125, 0.125, 0.125 }, { 0.125, 0, 0.125 }, { 0.125, 0.125, 0.125 } };
	cv::Mat filterKernel(3, 3, CV_32F, m);
	cv::filter2D(tmp, dst, CV_8U , filterKernel, cv::Point(1, 1));
	cv::threshold(dst, dst, 95, 255, cv::THRESH_BINARY);

	// invert
	dst = dst.mul(tmp);
	dst = cv::Scalar::all(255) - dst;

	cv::Point anchor(1, 1);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), anchor);
	cv::erode(dst, tmp, element, anchor, 2);
	cv::dilate(tmp, dst, element, anchor, 1);

	typedef std::vector<cv::Point2f> Line;
	typedef std::vector<std::shared_ptr<Line>> LineArray;

	LineComputer line;
	LineArray calibrationArray;
	calibrationArray.reserve(1024);
	cv::Point2f lastCentroid(-1, -1);

	// start scanning at the top left corner
	for (int row = 0; row < dst.rows; ++row) {
		std::uint8_t* pixel = dst.ptr<std::uint8_t>(row);
		for (int col = 0; col < dst.cols; ++col) {
			// if the pixel is not black and it is not marked, process
			if (pixel[col] == MASK_OBJECT) {
				cv::Point2f centroid = findCentroid(greyMat, dst, cv::Point2i(col, row));
				centroid = refineCentroid(greyMat, centroid);
				dst.at<uchar>(centroid) = 192;

				if (lastCentroid.x < 0) {
					line.addPoint(centroid);
				}
				else {
					double expected = line.expect(centroid.x);
					if ( (std::abs(centroid.y - expected) < 2.0 ) && (std::abs(centroid.x - lastCentroid.x) < 500 )) {
						line.addPoint(centroid);
					}
					else {
						// store the line and create a new one
						line.sortLine();
						calibrationArray.push_back(line.getLine());
						line = LineComputer();
						line.addPoint(centroid);
					}
				}

				lastCentroid = centroid;
			}
		}
	}
	calibrationArray.push_back(line.getLine());

	// DEBUG: draw lines
	/*dst = cv::Scalar(256, 256, 256);
	for (auto line : calibrationArray) {
		for (std::size_t i = 1; i < line->size(); ++i) {
			cv::line(dst, line->at(i-1), line->at(i), cv::Scalar(0, 0, 0));
		}
	}*/


	// DEBUG: convert to the format expected for viewwing
	dst.convertTo(tmp, CV_16U, 256);
	cv::cvtColor(tmp, pimpl->calibrationImage, CV_GRAY2RGB);
}

cv::Mat &Calibrator::getcalibrationImage() const {
	return pimpl->calibrationImage;
}

}
