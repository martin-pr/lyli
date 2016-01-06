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
#include <utility>
#include <vector>

#include <calibration/calibrationdata.h>

namespace cv {
	class Mat;
}

namespace Lyli {
namespace Image {
	class Metadata;
}
}

namespace Lyli {
namespace Calibration {

class PointGrid;


/**
 * A class providing means to calibrate camera from a set of images.
 */
class Calibrator {
public:
	class LensConfiguration {
	public:
		LensConfiguration(int zoom, int focus);
		int getZoomStep() const;
		int getFocusStep() const;
	private:
		int zoomStep;
		int focusStep;
	};

	using CalibrationResult = std::pair<LensConfiguration, CalibrationData>;

	Calibrator();
	~Calibrator();

	/**
	 * Add a grid to the calibrator and process it.
	 *
	 * @param pointgrid grid with lens centroids
	 * @param metadata of the image corresponding to the pointgrid
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
	std::vector<CalibrationResult> calibrate();

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
