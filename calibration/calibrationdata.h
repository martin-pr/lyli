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
#ifndef LYLI_CALIBRATION_CALIBRATIONDATA_H_
#define LYLI_CALIBRATION_CALIBRATIONDATA_H_

#include <memory>
#include <utility>
#include <vector>

namespace cv {
	class Mat;

	template < typename _Tp, int cn>
	class Vec;
	typedef Vec<float, 2> Vec2f;
}

namespace Json {
	class Value;
}

namespace Lyli {
namespace Calibration {

class LineGrid;

/**
 * Calibration data for the lens array.
 *
 * The lens array parameters stay the same for all lens configuration.
 */
class ArrayParameters {
public:
	ArrayParameters();
	ArrayParameters(const LineGrid& grid, const cv::Vec2f& translation, double rotation);
	~ArrayParameters();

	ArrayParameters(const ArrayParameters& other);
	ArrayParameters& operator=(const ArrayParameters& other);
	ArrayParameters(ArrayParameters&& other) noexcept;
	ArrayParameters& operator=(ArrayParameters&& other) noexcept;

	const LineGrid& getGrid() const;
	const cv::Vec2f& getTranslation() const;
	double getRotation() const;

	/**
	 * Serialize into a JSON object
	 * \return JSON object representing the class
	 */
	Json::Value serialize() const;
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

/**
 * Calibration data for the camera lens.
 *
 * The camera parameters depend on a lens configuration.
 */
class LensParameters {
public:
	LensParameters();
	LensParameters(const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs);
	~LensParameters();

	LensParameters(const LensParameters& other);
	LensParameters& operator=(const LensParameters& other);
	LensParameters(LensParameters&& other) noexcept;
	LensParameters& operator=(LensParameters&& other) noexcept;

	const cv::Mat& getCameraMatrix() const;
	const cv::Mat& getDistCoeffs() const;

	/**
	 * Serialize into a JSON object
	 * \return JSON object representing the class
	 */
	Json::Value serialize() const;
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

/**
 * A specific lens configuration
 */
class LensConfiguration {
public:
	LensConfiguration();
	LensConfiguration(int zoom, int focus);
	~LensConfiguration();

	LensConfiguration(const LensConfiguration& other);
	LensConfiguration& operator=(const LensConfiguration& other);
	LensConfiguration(LensConfiguration&& other) noexcept;
	LensConfiguration& operator=(LensConfiguration&& other) noexcept;

	int getZoomStep() const;
	int getFocusStep() const;

	/**
	 * Serialize into a JSON object
	 * \return JSON object representing the class
	 */
	Json::Value serialize() const;
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

/**
 * Holds all calibration data.
 */
class CalibrationData {
public:
	using LensConfigPair = std::pair<LensConfiguration, LensParameters>;
	using LensCalibration = std::vector<LensConfigPair>;

	CalibrationData();
	CalibrationData(const ArrayParameters& array, const LensCalibration& lens);
	~CalibrationData();

	CalibrationData(const CalibrationData& other);
	CalibrationData& operator=(const CalibrationData& other);
	CalibrationData(CalibrationData&& other) noexcept;
	CalibrationData& operator=(CalibrationData&& other) noexcept;

	const ArrayParameters& getArray() const;
	const LensCalibration& getLens() const;

	/**
	 * Serialize into a JSON object
	 * \return JSON object representing the class
	 */
	Json::Value serialize() const;
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

}
}

#endif
