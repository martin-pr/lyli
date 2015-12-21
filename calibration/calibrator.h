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

#include <cstdint>
#include <memory>
#include <vector>

#include <opencv2/core/core.hpp>

namespace cv {
	class Mat;
}

namespace Lyli {
namespace Calibration {

/**
 * Defines constants for the contents of the mask.
 */
struct Mask {
	static constexpr std::uint8_t EMPTY = 0;
	static constexpr std::uint8_t PROCESSED = 128;
	static constexpr std::uint8_t OBJECT = 255;
};

class LineGrid;

/**
 * Interface for the image preprocessing and the mask creation
 */
class PreprocessorInterface {
public:
	/**
	 * A default constructor.
	 */
	PreprocessorInterface();
	/**
	 * A destructor.
	 */
	virtual ~PreprocessorInterface();

	/**
	 * Preprocess the image to create a mask for the Calibrator.
	 *
	 * @param gray grayscale image to process
	 * @param outMask output mask for the calibrator using the constants from the Mask struct.
	 */
	virtual void preprocess(const cv::Mat &gray, cv::Mat &outMask) = 0;

	// avoid copying
	PreprocessorInterface(const PreprocessorInterface&) = delete;
	PreprocessorInterface& operator=(const PreprocessorInterface&) = delete;
};

/**
 * Interface for the lens detection
 */
class LensDetectorInterface {
public:
	/**
	 * A default constructor.
	 */
	LensDetectorInterface();
	/**
	 * A destructor
	 */
	virtual ~LensDetectorInterface();

	/**
	 * Detect lens centroids and put them in the line map.
	 *
	 * @param gray grayscale image to process
	 * @param mask mask with lenses set as Mask::OBJECT
	 * @return lens centroids grouped in lines
	 */
	virtual LineGrid detect(const cv::Mat &gray, cv::Mat &mask) = 0;

	// avoid copying
	LensDetectorInterface(const LensDetectorInterface&) = delete;
	LensDetectorInterface& operator=(const LensDetectorInterface&) = delete;
};

/**
 * Interface for filtering invalid lens centroids.
 */
class LensFilterInterface {
public:
	/**
	 * A default constructor.
	 */
	LensFilterInterface();
	/**
	 * A destructor
	 */
	virtual ~LensFilterInterface();

	/**
	 * Process the lines, remove the lines that do not spawn the entire image
	 * and ensure there are no invalid lens centroids and that no centroids are missing.
	 *
	 * @param lines lines to filter
	 * @return filtered lines
	 */
	virtual LineGrid filter(const LineGrid &lines) = 0;

	// avoid copying
	LensFilterInterface(const LensFilterInterface&) = delete;
	LensFilterInterface& operator=(const LensFilterInterface&) = delete;
};

class CalibrationData {
public:
	CalibrationData();
	CalibrationData(const cv::Mat &cameraMatrix, const cv::Mat &distCoeffs, const cv::Mat &translation, const cv::Mat &rotation);
	~CalibrationData();

	CalibrationData(const CalibrationData &other);
	CalibrationData& operator=(const CalibrationData &other);
	CalibrationData(CalibrationData &&other) noexcept;
	CalibrationData& operator=(CalibrationData &&other) noexcept;

	cv::Mat& getCameraMatrix() const;
	cv::Mat& getDistCoeffs() const;
	cv::Mat& getTranslation() const;
	cv::Mat& getRotation() const;

private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
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
	CalibrationData& getCalibrationData() const;

private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

}
}

#endif
