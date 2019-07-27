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

#ifndef LYLI_CALIBRATION_LENSDETECTORIFACE_H_
#define LYLI_CALIBRATION_LENSDETECTORIFACE_H_

#include <cstdint>
#include <memory>

namespace cv {
class Mat;
}

namespace Lyli {
namespace Calibration {

class PointGrid;

/**
 * Interface for the lens detection
 */
class LensDetectorInterface {
public:
	/**
	 * A default constructor.
	 */
	LensDetectorInterface() = default;
	/**
	 * A destructor
	 */
	virtual ~LensDetectorInterface() = default;

	/**
	 * Detect lens centroids and put them in the line map.
	 *
	 * @param image image to process
	 * @return pointgrid with lens centroids
	 */
	virtual PointGrid detect(const cv::Mat& image) = 0;

	// avoid copying
	LensDetectorInterface(const LensDetectorInterface&) = delete;
	LensDetectorInterface& operator=(const LensDetectorInterface&) = delete;
};

}
}

#endif
