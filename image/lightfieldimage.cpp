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

#include "lightfieldimage.h"

#include <cmath>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

#include <calibration/calibrationdata.h>
#include <calibration/linegrid.h>
#include <calibration/subgrid.h>
#include "metadata.h"
#include "rawimage.h"

namespace Lyli {
namespace Image {

class LightfieldImage::Impl {
public:
	cv::Mat image;
};

LightfieldImage::LightfieldImage(const Lyli::Image::RawImage& rawImage, const Lyli::Image::Metadata& metadata, const Calibration::CalibrationData& calibrationData) :
	pimpl(new Impl) {

	// transform input according to the calibration data
	cv::Mat tmp;
	rawImage.getData().copyTo(tmp);
	double angle = calibrationData.getArray().getRotation()*180.0/M_PI;
	cv::Mat r = cv::getRotationMatrix2D(cv::Point2f(0, 0), angle, 1.0);
	cv::Mat t = cv::Mat::eye(3 , 3, CV_64F);
	t.at<double>(0, 2) = calibrationData.getArray().getTranslation()[0];
	t.at<double>(1, 2) = calibrationData.getArray().getTranslation()[1];
	cv::Mat T = r*t;
	cv::warpAffine(tmp, tmp, T, tmp.size());

	// reserve space
	pimpl->image.create(calibrationData.getArray().getGrid().getHorizontalLines().size(), calibrationData.getArray().getGrid().getVerticalLines().size() / 2, CV_16UC3);

	// store color from each intersection in subgrid A
	const Calibration::LineGrid::LineList& horizontal(calibrationData.getArray().getGrid().getHorizontalLines());
	const Calibration::LineGrid::LineList& vertical(calibrationData.getArray().getGrid().getVerticalLines());
	for (std::size_t y = 0; y < vertical.size(); y++) {
		for (std::size_t x = 0; x < horizontal.size(); x++) {
			if(horizontal[x].subgrid == vertical[y].subgrid) {
				pimpl->image.at<cv::Vec3w>(cv::Point(y/2, x)) = tmp.at<cv::Vec3w>(cv::Point(vertical[y].position, horizontal[x].position));
			}
		}
	}
}

LightfieldImage::~LightfieldImage() {

}

const cv::Mat & LightfieldImage::getData() const {
	return pimpl->image;
}


}
}
