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

#ifndef LYLI_IMAGE_LIGHTFIELDIMAGE_H
#define LYLI_IMAGE_LIGHTFIELDIMAGE_H

#include <memory>

namespace cv {
class Mat;
}

namespace Lyli {

namespace Calibration {
class CalibrationData;
}

namespace Image {

class Metadata;
class RawImage;

class LightfieldImage {
public:
	LightfieldImage(const RawImage& rawImage, const Metadata& metadata, const Calibration::CalibrationData& calibrationData);
	~LightfieldImage();

	// DEBUG
	const cv::Mat &getData() const;

private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

}
}

#endif // LYLI_IMAGE_LIGHTFIELDIMAGE_H
