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
 */

#ifndef LYLI_CALIBRATION_SERIALIZATION_H_
#define LYLI_CALIBRATION_SERIALIZATION_H_

#include <json/value.h>

#include <opencv2/core/core.hpp>

namespace Lyli {
namespace Calibration {

/**
 * Serialize cv::Mat
 */
Json::Value serialize(const cv::Mat& mat);

/**
 * Serialize cv::Vec*
 */
template<typename T, int cn>
Json::Value serialize(const cv::Vec<T, cn>& vec) {
	Json::Value root(Json::arrayValue);
	for (int i = 0; i < cn; ++i) {
		root[i] = vec[i];
	}
	return root;
}

}
}

#endif
