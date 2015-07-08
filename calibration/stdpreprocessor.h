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

#ifndef LYLI_CALIBRATION_PREPROCESSOR_H_
#define LYLI_CALIBRATION_PREPROCESSOR_H_

#include "calibrator.h"

namespace Lyli {
namespace Calibration {

class StdPreprocessor : public Preprocessor {
public:
	/**
	 * Preprocess the image to create a mask for the Calibrator.
	 *
	 * @param gray grayscale image to process
	 * @param outMask output mask for the calibrator using the values from the Mask enum.
	 */
	void preprocess(const cv::Mat &gray, cv::Mat &outMask) override;
};

}
}

#endif

