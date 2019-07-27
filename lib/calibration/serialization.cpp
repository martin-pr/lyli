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
#include "serialization.h"

#include <cassert>

#include <json/value.h>

#include <opencv2/core/core.hpp>

namespace Lyli {
namespace Calibration {

Json::Value serialize(const cv::Mat& mat) {
	// for now handle only double 2-dimensional matrices
	assert(mat.dims == 2);
	assert(mat.type() == CV_64F);

	Json::Value root(Json::arrayValue);
	for (int i = 0; i < mat.rows; ++i) {
		root[i] = Json::Value(Json::arrayValue);
		for (int j = 0; j < mat.cols; ++j) {
			root[i][j] = mat.at<double>(i, j);
		}
	}
	return root;
}

void deserialize(const Json::Value& value, cv::Mat& mat) {
	assert(value.isArray());
	assert(!value.empty() && value[0].isArray());

	mat = cv::Mat(value.size(), value[0].size(), CV_64F);
	for (int i = 0; i < mat.rows; ++i) {
		for (int j = 0; j < mat.cols; ++j) {
			mat.at<double>(i, j) = value[i][j].asDouble();
		}
	}
}

}
}
