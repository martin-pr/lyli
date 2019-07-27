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

#ifndef LYLI_CALIBRATION_LENSDETECTOR_H_
#define LYLI_CALIBRATION_LENSDETECTOR_H_

#include <cstdint>
#include <memory>

#include <calibration/lensdetectoriface.h>

namespace cv {
class Mat;
}

namespace Lyli {
namespace Calibration {

class PointGrid;

/**
 * Defines constants for the contents of the mask.
 */
struct Mask {
	static constexpr std::uint8_t EMPTY = 0;
	static constexpr std::uint8_t PROCESSED = 128;
	static constexpr std::uint8_t OBJECT = 255;
};

/**
 * Interface for the image preprocessing and the mask creation
 */
class PreprocessorInterface {
public:
	/**
	 * A default constructor.
	 */
	PreprocessorInterface() = default;
	/**
	 * A destructor.
	 */
	virtual ~PreprocessorInterface() = default;

	/**
	 * Preprocess the image to create a mask for the Calibrator.
	 *
	 * @param gray grayscale image to process
	 * @return output mask for the calibrator using the constants from the Mask struct.
	 */
	virtual cv::Mat preprocess(const cv::Mat &gray) = 0;

	// avoid copying
	PreprocessorInterface(const PreprocessorInterface&) = delete;
	PreprocessorInterface& operator=(const PreprocessorInterface&) = delete;
};

/**
 * Lens detector that detected lenses as centroids of objects in a grayscale mask
 */
class LensDetector : public LensDetectorInterface {
public:
	LensDetector(std::unique_ptr<PreprocessorInterface> preprocessor);
	PointGrid detect(const cv::Mat& image) override;

private:
	std::unique_ptr<PreprocessorInterface> preprocessor;
};

}
}

#endif
