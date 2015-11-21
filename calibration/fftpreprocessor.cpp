
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

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace {

// radius where frequencies are smoothed
constexpr int BLUR_RADIUS = 10;
// BLUR_RADIUS^2
constexpr int BLUR_RADIUS2 = 100;

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

	// flatten the lowest frequencies by blurring pixels in the circle over lowest frequencies
	// just stupid box blur
	cv::Mat center;
	cv::split(complexI, planes);
	for (int i = 0; i < 2; ++i) {
		for (int blur = 0; blur < 5; ++blur) {
			for (int y = BLUR_RADIUS; y >= -BLUR_RADIUS; --y) {
				int realy_a = cv::borderInterpolate(y - 1, planes[i].rows, cv::BORDER_WRAP);
				int realy_b = cv::borderInterpolate(y, planes[i].rows, cv::BORDER_WRAP);
				int realy_c = cv::borderInterpolate(y + 1, planes[i].rows, cv::BORDER_WRAP);
				int x0 = std::round(std::sqrt(BLUR_RADIUS2 - y*y));
				for (int x = -x0; x <= x0; ++x) {
					int realx_a = cv::borderInterpolate(x - 1, planes[i].cols, cv::BORDER_WRAP);
					int realx_b = cv::borderInterpolate(x, planes[i].cols, cv::BORDER_WRAP);
					int realx_c = cv::borderInterpolate(x + 1, planes[i].cols, cv::BORDER_WRAP);

					planes[i].at<float>(realy_b, realx_b) = (
						planes[i].at<float>(realy_a, realx_b)
						+ planes[i].at<float>(realy_b, realx_a) + planes[i].at<float>(realy_b, realx_b) + planes[i].at<float>(realy_b, realx_c)
						+ planes[i].at<float>(realy_c, realx_b)
						) / 6.0;
				}
			}
		}
	}
	cv::merge(planes, 2, complexI);

	/*cv::magnitude(planes[0], planes[1], planes[0]);
	cv::Mat magI = planes[0];
	magI += cv::Scalar::all(1);
	cv::log(magI, magI);
	int cx = magI.cols/2; /////
	int cy = magI.rows/2;
	cv::Mat q0(magI, cv::Rect(0, 0, cx, cy));
	cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));
	cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));
	cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy));
	cv::Mat tmp;
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2); /////
	cv::normalize(magI, magI, 0, 1, CV_MINMAX);
	outMask = magI;
	magI.convertTo(outMask, CV_8U, 255);
	return;//*/

	// inverse transform
	cv::Mat invDFT;
	cv::idft(complexI, invDFT, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT );

	// normalize the values and convert to uint8 to ensure the values are in 0-255 scale
	cv::normalize(invDFT, invDFT, 0, 1, CV_MINMAX);
	invDFT.convertTo(outMask, CV_8U, 255);

	// apply threshold
	cv::threshold(outMask, outMask, 128, 255, cv::THRESH_BINARY);
}

}
}
