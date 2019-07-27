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

#include <memory>

#include <calibration/calibrationdata.h>
#include <calibration/exception.h>

namespace Lyli {
namespace Image {
class Metadata;
}
}

namespace Lyli {
namespace Calibration {

class PointGrid;

class CameraDiffersException : public Exception {
public:
	explicit CameraDiffersException(const std::string& reason);
	virtual ~CameraDiffersException();

	virtual const char* what() const noexcept;

private:
	std::string m_reason;
};

/**
 * A class providing means to calibrate camera from a set of images.
 */
class Calibrator {
public:
	Calibrator();
	~Calibrator();

	/**
	 * Add a grid to the calibrator and process it.
	 *
	 * The metada must correspond to the same camera as all previously added metada.
	 *
	 * @param pointgrid grid with lens centroids
	 * @param metadata of the image corresponding to the pointgrid
	 * @throw CameraDiffersException in case the added metada are for a different camera
	 */
	void addGrid(const PointGrid &pointgrid, const Lyli::Image::Metadata &metadata);

	/**
	 * Finish the calibration.
	 *
	 * Computes the calibration data from all previously supplied images.
	 * This function may take long time to finish.
	 *
	 * @return the calibration data
	 */
	CalibrationData calibrate();

	/**
	 * Reset the calibrator state.
	 *
	 * The stored state of any processed images is thrown away after calling this function, allowing
	 * the calibrator to be reused for new set of images.
	 */
	void reset();

private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

}
}

#endif
