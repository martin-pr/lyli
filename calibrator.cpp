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

#include <cstdint>

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

		if (data[pos] > 0) {
			// fill to the left
			int tmppos = pos;
			int oldstartx = startx;
			int x = startx - 1;
			while(x >= 0 && data[tmppos - 1] > 0) {
				// compute
				m10 += y * data[tmppos];
				m01 += x * data[tmppos];
				sum += data[tmppos];
				maskData[tmppos] = 255;
				// move to next
				--tmppos;
				--startx;
				--x;
			}
			// fill to the right
			tmppos = pos;
			x = oldstartx;
			while(x < image.cols && data[tmppos] > 0) {
				// compute
				m10 += y * data[tmppos];
				m01 += x * data[tmppos];
				sum += data[tmppos];
				maskData[tmppos] = 255;
				// move to next
				++tmppos;
				++x;
			}
			endx = x - 1;
		}
		else {
			// find the start position
			int tmppos = pos;
			while(data[tmppos] == 0) {
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
			while(x < image.cols && data[tmppos] > 0) {
				// compute
				m10 += y * data[tmppos];
				m01 += x * data[tmppos];
				sum += data[tmppos];
				maskData[tmppos] = 255;
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

	return cv::Point2f(m10/sum, m01/sum);
}

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
	cv::Mat dst, greyMat, tmp, res;

	// convert to gray
	cv::cvtColor(pimpl->image, greyMat, CV_RGB2GRAY);
	greyMat.convertTo(greyMat, CV_8U, 1.0/256.0);

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

	// start scanning at the top left corner
	// first create a mask that we will use to makr processed pixels
	cv::Mat processedMask(cv::Mat::zeros(dst.rows, dst.cols, CV_8U));
	for (int row = 0; row < dst.rows; ++row) {
		std::uint8_t* pixel = dst.ptr<std::uint8_t>(row);
		std::uint8_t* maskPixel = processedMask.ptr<std::uint8_t>(row);
		for (int col = 0; col < dst.cols; ++col) {
			// if the pixel is not black and it is not marked, process
			if (pixel[col] != 0 && maskPixel[col] == 0) {
				cv::Point2f centroid = findCentroid(dst, processedMask, cv::Point2i(col, row));
				dst.at<uchar>(centroid.x, centroid.y) = 128;
			}
		}
	}

	// DEBUG: convert to the format expected for viewwing
	dst.convertTo(res, CV_16U, 256);
	cv::cvtColor(res, pimpl->calibrationImage, CV_GRAY2RGB);
}

cv::Mat &Calibrator::getcalibrationImage() const {
	return pimpl->calibrationImage;
}

}
