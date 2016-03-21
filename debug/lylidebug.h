/*
 * This file is part of Lyli, an application to control Lytro camera
 * Copyright (C) 2016  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
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
#ifndef LYLI_DEBUG_DEBUG_H_
#define LYLI_DEBUG_DEBUG_H_

#include <sstream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <calibration/linegrid.h>
#include <calibration/subgrid.h>

namespace Lyli {
namespace Debug {

/**
 * Draw lineGrid
 * \param file the output image
 * \param lineGrid lineGrid to draw
 */
inline void drawLineGrid(const char* file, const Lyli::Calibration::LineGrid &lineGrid) {
	cv::Mat dst(3280, 3280, CV_8UC3);
	dst = cv::Scalar(256, 256, 256);
	const auto &linesHorizontal = lineGrid.getHorizontalLines();
	for (const auto &line : linesHorizontal) {
		cv::Scalar color = line.subgrid == Lyli::Calibration::SubGrid::SUBGRID_A ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 255, 0);
		cv::line(dst, cv::Point2f(0, line.position), cv::Point2f(3280, line.position), color);
	}
	const auto &linesVertical = lineGrid.getVerticalLines();
	for (const auto &line : linesVertical) {
		cv::Scalar color = line.subgrid == Lyli::Calibration::SubGrid::SUBGRID_A ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 255, 0);
		cv::line(dst, cv::Point2f(line.position, 0), cv::Point2f(line.position, 3280), color);
	}

	cv::imwrite(file, dst);
}

/**
 * Draw lineGrid overload
 * \param fileBase the output file name base
 * \param index index appended to the filebase
 * \param ext extension
 * \param lineGrid lineGrid to draw
 */
inline void drawLineGrid(const char* fileBase, int index, const char* ext, const Lyli::Calibration::LineGrid &lineGrid) {
	std::stringstream ss;
	ss << fileBase << index << ext;
	drawLineGrid(ss.str().c_str(), lineGrid);
}

}
}

#endif
