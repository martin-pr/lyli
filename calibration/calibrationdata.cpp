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
#include "calibrationdata.h"

#include <algorithm>
#include <utility>

#include "linegrid.h"
#include "serialization.h"

#include <json/value.h>

#include <opencv2/core/core.hpp>

namespace Lyli {
namespace Calibration {

/***********************
 * ArrayParameters *
 ***********************/
class ArrayParameters::Impl {
public:
	Impl() : m_rotation(0.0) {

	}
	Impl(const LineGrid& grid, const cv::Vec2f& translation, double rotation) : m_grid(grid), m_translation(translation), m_rotation(rotation) {

	}

	LineGrid m_grid;
	cv::Vec2f m_translation;
	double m_rotation;
};

ArrayParameters::ArrayParameters() : pimpl(new Impl) {

}

ArrayParameters::ArrayParameters(const LineGrid& grid, const cv::Vec2f& translation, double rotation) : pimpl(new Impl(grid, translation, rotation)) {

}

ArrayParameters::~ArrayParameters() {

}

ArrayParameters::ArrayParameters(const ArrayParameters& other) :
	pimpl(new Impl(other.pimpl->m_grid, other.pimpl->m_translation, other.pimpl->m_rotation)) {

}

ArrayParameters& ArrayParameters::operator=(const ArrayParameters& other) {
	if (this != &other) {
		ArrayParameters tmp(other);
		std::swap(tmp.pimpl, pimpl);
	}
	return *this;
}

ArrayParameters::ArrayParameters(ArrayParameters&& other) noexcept : pimpl(std::move(other.pimpl)) {

}

ArrayParameters& ArrayParameters::operator=(ArrayParameters&& other) noexcept {
	if (this != &other) {
		pimpl = std::move(other.pimpl);
	}
	return *this;
}

const LineGrid& ArrayParameters::getGrid() const {
	return pimpl->m_grid;
}

const cv::Vec2f& ArrayParameters::getTranslation() const {
	return pimpl->m_translation;
}

double ArrayParameters::getRotation() const {
	return pimpl->m_rotation;
}

Json::Value ArrayParameters::serialize() const {
	Json::Value root(Json::objectValue);
	root["translation"] = ::Lyli::Calibration::serialize(pimpl->m_translation);
	root["rotation"] = pimpl->m_rotation;
	root["grid"] = pimpl->m_grid.serialize();
	return root;
}

void ArrayParameters::deserialize(const Json::Value& value) {
	::Lyli::Calibration::deserialize(value["translation"], pimpl->m_translation);
	pimpl->m_rotation = value["rotation"].asDouble();
	pimpl->m_grid.deserialize(value["grid"]);
}

/*******************
 * LensParameters *
 *******************/
class LensParameters::Impl {
public:
	Impl() {

	}
	Impl(const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs) : m_cameraMatrix(cameraMatrix), m_distCoeffs(distCoeffs) {

	}

	cv::Mat m_cameraMatrix;
	cv::Mat m_distCoeffs;
};

LensParameters::LensParameters() : pimpl(new Impl) {

}

LensParameters::LensParameters(const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs) : pimpl(new Impl(cameraMatrix, distCoeffs)) {

}

LensParameters::~LensParameters() {

}

LensParameters::LensParameters(const LensParameters& other) :
	pimpl(new Impl(other.pimpl->m_cameraMatrix, other.pimpl->m_distCoeffs)) {

}

LensParameters& LensParameters::operator=(const LensParameters& other) {
	if (this != &other) {
		LensParameters tmp(other);
		std::swap(tmp.pimpl, pimpl);
	}
	return *this;
}

LensParameters::LensParameters(LensParameters&& other) noexcept : pimpl(std::move(other.pimpl)) {

}

LensParameters& LensParameters::operator=(LensParameters&& other) noexcept {
	if (this != &other) {
		pimpl = std::move(other.pimpl);
	}
	return *this;
}

const cv::Mat& LensParameters::getCameraMatrix() const {
	return pimpl->m_cameraMatrix;
}

const cv::Mat& LensParameters::getDistCoeffs() const {
	return pimpl->m_distCoeffs;
}

Json::Value LensParameters::serialize() const {
	Json::Value root(Json::objectValue);
	root["cameraMatrix"] = ::Lyli::Calibration::serialize(pimpl->m_cameraMatrix);
	root["distCoeffs"] = ::Lyli::Calibration::serialize(pimpl->m_distCoeffs);
	return root;
}

void LensParameters::deserialize(const Json::Value& value) {
	::Lyli::Calibration::deserialize(value["cameraMatrix"], pimpl->m_cameraMatrix);
	::Lyli::Calibration::deserialize(value["distCoeffs"], pimpl->m_distCoeffs);
}

/********************
 * LensConfiguration *
 ********************/
class LensConfiguration::Impl {
public:
	Impl() {

	}
	Impl(int zoom, int focus) : m_zoom(zoom), m_focus(focus) {

	}

	int m_zoom;
	int m_focus;
};

LensConfiguration::LensConfiguration() : pimpl(new Impl) {

}

LensConfiguration::LensConfiguration(int zoom, int focus) : pimpl(new Impl(zoom, focus)) {

}

LensConfiguration::~LensConfiguration() {

}

LensConfiguration::LensConfiguration(const LensConfiguration& other) :
	pimpl(new Impl(other.pimpl->m_zoom, other.pimpl->m_focus)) {

}

LensConfiguration& LensConfiguration::operator=(const LensConfiguration& other) {
	if (this != &other) {
		LensConfiguration tmp(other);
		std::swap(tmp.pimpl, pimpl);
	}
	return *this;
}

LensConfiguration::LensConfiguration(LensConfiguration&& other) noexcept : pimpl(std::move(other.pimpl)) {

}

LensConfiguration& LensConfiguration::operator=(LensConfiguration&& other) noexcept {
	if (this != &other) {
		pimpl = std::move(other.pimpl);
	}
	return *this;
}

int LensConfiguration::getZoomStep() const {
	return pimpl->m_zoom;
}

int LensConfiguration::getFocusStep() const {
	return pimpl->m_focus;
}

Json::Value LensConfiguration::serialize() const {
	Json::Value root(Json::objectValue);
	root["zoomStep"] = pimpl->m_zoom;
	root["focusStep"] = pimpl->m_focus;
	return root;
}

void LensConfiguration::deserialize(const Json::Value& value) {
	pimpl->m_zoom = value["zoomStep"].asDouble();
	pimpl->m_focus = value["focusStep"].asDouble();
}

/*******************
 * CalibrationData *
 *******************/
class CalibrationData::Impl {
public:
	Impl() {

	}
	Impl(const ArrayParameters& array, const LensCalibration& lens) : m_array(array), m_lens(lens) {
		// sort the lens parameters
		std::sort(m_lens.begin(), m_lens.end(),
			[](const auto &a, const auto &b) {
				if(a.first.getZoomStep() < b.first.getZoomStep())
					return true;
				else if (a.first.getZoomStep() == b.first.getZoomStep())
					return a.first.getFocusStep() < b.first.getFocusStep();
				return false;
			});
	}

	ArrayParameters m_array;
	LensCalibration m_lens;
};

CalibrationData::CalibrationData() : pimpl(new Impl) {

}

CalibrationData::CalibrationData(const ArrayParameters& array, const LensCalibration& lens) :
	pimpl(new Impl(array, lens)) {

}

CalibrationData::~CalibrationData() {

}

CalibrationData::CalibrationData(const CalibrationData& other) :
	pimpl(new Impl(other.pimpl->m_array, other.pimpl->m_lens)) {

}

CalibrationData& CalibrationData::operator=(const CalibrationData& other) {
	if (this !=& other) {
		CalibrationData tmp(other);
		std::swap(pimpl, tmp.pimpl);
	}
	return *this;
}

CalibrationData::CalibrationData(CalibrationData&& other) noexcept : pimpl(std::move(other.pimpl)) {

}

CalibrationData& CalibrationData::operator=(CalibrationData&& other) noexcept {
	if (this !=& other) {
		pimpl = std::move(other.pimpl);
	}
	return *this;
}

const ArrayParameters& CalibrationData::getArray() const {
	return pimpl->m_array;
}

const CalibrationData::LensCalibration& CalibrationData::getLens() const {
	return pimpl->m_lens;
}

Json::Value CalibrationData::serialize() const {
	Json::Value root(Json::objectValue);
	root["array"] = pimpl->m_array.serialize();
	root["lens"] = Json::Value(Json::arrayValue);
	for (std::size_t i = 0; i < pimpl->m_lens.size(); ++i) {
		root["lens"][static_cast<int>(i)]["configuration"] = pimpl->m_lens[i].first.serialize();
		root["lens"][static_cast<int>(i)]["parameters"] = pimpl->m_lens[i].second.serialize();
	}
	return root;
}

void CalibrationData::deserialize(const Json::Value& value) {
	pimpl->m_array.deserialize(value["array"]);
	const Json::Value& lensRoot = value["lens"];
	for (Json::Value::ArrayIndex i = 0, end = lensRoot.size(); i < end; ++i) {
		LensConfiguration config;
		config.deserialize(lensRoot[i]["configuration"]);
		LensParameters array;
		array.deserialize(lensRoot[i]["parameters"]);
		pimpl->m_lens.push_back(std::make_pair(config, array));
	}
}

}
}
