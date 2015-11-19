
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

#include "fftpreprocessor.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace {

// optimal size as computed using cv::getOptimalDFTSize for the resolution 3280x3280
constexpr int DFT_OPTIMAL_SIZE = 3375;

}

namespace Lyli {
namespace Calibration {

void FFTPreprocessor::preprocess(const cv::Mat& gray, cv::Mat& outMask) {
	cv::Mat padded;
	cv::copyMakeBorder(gray, padded, 0, DFT_OPTIMAL_SIZE - gray.rows, 0, DFT_OPTIMAL_SIZE - gray.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	// create an matrix that that has two channels - one for real part and one imaginary part of DFT
	cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
	cv::Mat complexI;
	cv::merge(planes, 2, complexI);

	// do the transform
	cv::dft(complexI, complexI);

	// flatten the lowest frequencies
	cv::Mat center;
	cv::split(complexI, planes);
	for (int i = 0; i < 2; ++i) {
		center = planes[i](cv::Rect(0, 0, 3, 3));
		cv::GaussianBlur(center, center, cv::Size(0, 0), 1, 1, cv::BORDER_REPLICATE);
		center = planes[i](cv::Rect(planes[i].cols - 3, 0, 3, 3));
		cv::GaussianBlur(center, center, cv::Size(0, 0), 1, 1, cv::BORDER_REPLICATE);
		center = planes[i](cv::Rect(planes[i].cols - 3, planes[i].rows - 3, 3, 3));
		cv::GaussianBlur(center, center, cv::Size(0, 0), 1, 1, cv::BORDER_REPLICATE);
		center = planes[i](cv::Rect(0, planes[i].rows - 3, 3, 3));
		cv::GaussianBlur(center, center, cv::Size(0, 0), 1, 1, cv::BORDER_REPLICATE);
	}
	cv::merge(planes, 2, complexI);

	/*cv::magnitude(planes[0], planes[1], planes[0]);
	cv::Mat magI = planes[0];
	magI += cv::Scalar::all(1);
    cv::log(magI, magI);
	cv::normalize(magI, magI, 0, 1, CV_MINMAX);
	outMask = magI;
	//magI.convertTo(outMask, CV_8U);//*/

	// inverse transform
	cv::Mat invDFT;
	cv::idft(complexI, invDFT, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT );
	invDFT(cv::Rect(0, 0, gray.cols, gray.rows)).convertTo(outMask, CV_8U);

	// apply threshold
	//cv::threshold(outMask, tmp, threshold, Mask::OBJECT, cv::THRESH_BINARY);
}

}
}
