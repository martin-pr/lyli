
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

#include <cmath>
#include <cstdint>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace {

// radius where frequencies are cut off
constexpr int HIGHPASS_CUTOFF = 10;
// HIGHPASS_CUTOFF^2
constexpr int HIGHPASS_CUTOFF_2 = 100;

}

namespace Lyli {
namespace Calibration {

void FFTPreprocessor::preprocess(const cv::Mat& gray, cv::Mat& outMask) {
	// create an matrix that that has two channels - one for real part and one imaginary part of DFT
	// note that I don't use the optimal size for DFT, as I was not able to make the lagorithm
	// work well when that was used
	cv::Mat planes[] = {cv::Mat_<float>(gray), cv::Mat::zeros(gray.size(), CV_32F)};
	cv::Mat complexI;
	cv::merge(planes, 2, complexI);

	// do the transform
	cv::dft(complexI, complexI);

	// the main part of the preprocess - remove all low frequency variations
	cv::split(complexI, planes);
	for (int i = 0; i < 2; ++i) {
		// remove all low frequencies
		for (int y = HIGHPASS_CUTOFF; y >= -HIGHPASS_CUTOFF; --y) {
			int realy = cv::borderInterpolate(y, planes[i].rows, cv::BORDER_WRAP);
			int x0 = std::round(std::sqrt(HIGHPASS_CUTOFF_2 - y*y));
			for (int x = -x0; x <= x0; ++x) {
				int realx = cv::borderInterpolate(x, planes[i].cols, cv::BORDER_WRAP);

				planes[i].at<float>(realy, realx) = 0.0;
			}
		}
	}
	cv::merge(planes, 2, complexI);

	// inverse transform
	cv::Mat invDFT;
	cv::idft(complexI, invDFT, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT );

	// normalize the values and convert to uint8 to ensure the values are in 0-255 scale
	cv::normalize(invDFT, invDFT, 0, 1, CV_MINMAX);
	invDFT.convertTo(outMask, CV_8U, 255);

	// apply threshold
	std::uint8_t threshold = cv::mean(outMask)[0] + 20;
	cv::threshold(outMask, outMask, threshold, 255, cv::THRESH_BINARY);

	// remove short spurs
	cv::Point anchor(1, 1);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), anchor);
	cv::morphologyEx(outMask, outMask, cv::MORPH_OPEN, kernel, anchor, 1, cv::BORDER_CONSTANT);
}

}
}
