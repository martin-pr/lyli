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

#ifndef LYLI_CALIBRATION_LINE_H_
#define LYLI_CALIBRATION_LINE_H_


#include <opencv2/core/core.hpp>
#include <vector>

namespace Lyli {
namespace Calibration {

/**
 * A line of lens centroids.
 */
using Line = std::vector<cv::Point2d>;
/**
 * A line of pointers to lens centroids.
 *
 * PtrLine is used when there are multiple lines that refer to the same lens,
 * such as in LineGrid.
 */
using PtrLine = std::vector<cv::Point2f*>;

}
}

#endif
