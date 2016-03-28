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

#include "lyliconfig.h"

#include <fstream>

#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QtCore/QString>

#include <iostream>

namespace {

QString getCalibrationPath(std::string serial) {
	QString cfgPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
	QDir cfgDir(cfgPath);
	if (!QFileInfo(cfgDir.path()).isReadable()) {
		return QString();
	}

	// calibration file location
	QString calibrationPath = cfgDir.filePath(QString::fromStdString(serial + ".json"));
	if (!QFileInfo(calibrationPath).isReadable()) {
		return QString();
	}

	return calibrationPath;
}

}

std::unique_ptr<::Lyli::Calibration::CalibrationData> LyliConfig::readCalibrationData(std::string serial) {
	// config location
	QString calibrationPath = getCalibrationPath(serial);
	if(calibrationPath.isEmpty()) {
		return nullptr;
	}

	// read calibration
	auto calibration = std::make_unique<::Lyli::Calibration::CalibrationData>();
	std::fstream fin(calibrationPath.toLocal8Bit().constData(), std::fstream::in | std::fstream::binary);
	Json::CharReaderBuilder readerbuilder;
	Json::Value root;
	Json::parseFromStream(readerbuilder, fin, &root, 0);
	calibration->deserialize(root);

	return calibration;
}

void LyliConfig::storeCalibrationData(const ::Lyli::Calibration::CalibrationData& calibration) {
	// ensure the config location exists
	QString cfgPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
	QDir cfgDir(cfgPath);
	if (!QFileInfo(cfgDir.path()).exists()) {
		cfgDir.mkpath(".");
	}
	// get calibration file path
	QString calibrationPath = cfgDir.filePath(QString::fromStdString(calibration.getSerial() + ".json"));

	Json::Value json = calibration.serialize();
	std::ofstream fout(calibrationPath.toLocal8Bit().constData(), std::fstream::out | std::fstream::trunc | std::fstream::binary);
	Json::StyledStreamWriter styledWriter;
	styledWriter.write(fout, json);
	fout.close();
}
