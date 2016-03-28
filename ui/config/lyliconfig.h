/*
 * This file is part of Lyli, an application to control Lytro camera
 * Copyright (C) 2016  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
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
 * 
 */

#ifndef LYLICONFIG_H
#define LYLICONFIG_H

#include <memory>
#include <string>

#include <calibration/calibrationdata.h>

class LyliConfig {
public:
	/**
	 * Read calibration data for a camera
	 * \param serial serial number of the camera
	 * \return calibration data or null pointer if the calibration data are not avaialable
	 */
	static std::unique_ptr<::Lyli::Calibration::CalibrationData> readCalibrationData(std::string serial);

	/**
	 * Store calibration data
	 */
	static void storeCalibrationData(const ::Lyli::Calibration::CalibrationData& calibration);
};

#endif // LYLICONFIG_H
