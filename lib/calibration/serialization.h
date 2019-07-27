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
 * \param mat Matrix to serialize
 * \return serialized value
 */
Json::Value serialize(const cv::Mat& mat);

/**
 * Deserialize cv::Mat
 * \param value JSON value to deserialize
 * \param[out] mat reference to matrix where the value is deserialized
 */
void deserialize(const Json::Value& value, cv::Mat& mat);

/**
 * Serialize cv::Vec*
 * \param vec vector to serialize
 * \return serialized value
 */
template<typename T, int cn>
Json::Value serialize(const cv::Vec<T, cn>& vec) {
	Json::Value root(Json::arrayValue);
	for (int i = 0; i < cn; ++i) {
		root[i] = vec[i];
	}
	return root;
}

/**
 * Deserialize cv::Vec*
 * \param value JSON value from which the vector is deserialized
 * \param[out] vec output vector
 */
template<int cn>
void deserialize(const Json::Value& value, cv::Vec<float, cn>& vec) {
	for (int i = 0; i < cn; ++i) {
		vec[i] = value[i].asFloat();
	}
}

template<int cn>
void deserialize(const Json::Value& value, cv::Vec<double, cn>& vec) {
	for (int i = 0; i < cn; ++i) {
		vec[i] = value[i].asDouble();
	}
}

}
}

#endif
