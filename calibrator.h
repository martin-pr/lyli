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

#ifndef LYLI_CALIBRATOR_H_
#define LYLI_CALIBRATOR_H_

#include <memory>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace Lyli {

/**
 * A class providing means to calibrate camera from a set of images.
 */
class Calibrator {
public:
	Calibrator();
	~Calibrator();

	void addImage(const cv::Mat &image);
	void calibrate();

	cv::Mat &getcalibrationImage() const;

private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

}

#endif
