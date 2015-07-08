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

#ifndef LYLI_CALIBRATION_CALIBRATOR_H_
#define LYLI_CALIBRATION_CALIBRATOR_H_

#include <ctype.h>
#include <memory>

namespace cv {
	class Mat;
}

namespace Lyli {
namespace Calibration {

/**
	* Defines constants for the contents of the mask.
	*/
struct Mask {
	static constexpr uint8_t EMPTY = 0;
	static constexpr uint8_t PROCESSED = 128;
	static constexpr uint8_t OBJECT = 255;
};

class Preprocessor {
public:
	/**
	 * A default constructor.
	 */
	Preprocessor();
	/**
	 * A destructor.
	 */
	virtual ~Preprocessor();

	/**
	 * Preprocess the image to create a mask for the Calibrator.
	 *
	 * @param gray grayscale image to process
	 * @param outMask output mask for the calibrator using the constants from the Mask struct.
	 */
	virtual void preprocess(const cv::Mat &gray, cv::Mat &outMask) = 0;

	// avoid copying
	Preprocessor(const Preprocessor&) = delete;
	Preprocessor& operator=(const Preprocessor&) = delete;
};

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
}

#endif
