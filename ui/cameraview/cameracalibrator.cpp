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

#include "cameracalibrator.h"

#include <atomic>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QFileInfoList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtWidgets/QProgressDialog>

#include <tbb/parallel_for_each.h>

#include <camera.h>
#include <calibration/calibrator.h>
#include <calibration/calibrationdata.h>
#include <calibration/fftpreprocessor.h>
#include <calibration/lensdetector.h>
#include <calibration/pointgrid.h>
#include <image/metadata.h>
#include <image/rawimage.h>
#include <filesystem/filelist.h>
#include <filesystem/filesystemaccess.h>
#include <filesystem/image.h>

#include <calibrationwizard/calibrationwizard.h>
#include <config/lyliconfig.h>

CameraCalibrator::CameraCalibrator(QObject *parent) : QObject(parent) {

}

CameraCalibrator::~CameraCalibrator() {

}

void CameraCalibrator::onCameraChanged(Lyli::Camera* camera) {
	if (!LyliConfig::readCalibrationData(camera->getCameraInformation().serial)) {
		CalibrationWizard wizard;
		wizard.exec();
		QString qserial(QString::fromStdString(camera->getCameraInformation().serial));
		QString cachePath(wizard.field("cacheDir").toString());
		QDir cacheDir(cachePath);

		// ensure the cachedir exists and create path based on serial
		if (!cacheDir.exists()) {
			cacheDir.mkpath(qserial);
		}
		cacheDir = cacheDir.filePath(qserial);

		// get list of files and prepare dialog
		Lyli::Filesystem::ImageList fileList(camera->getFilesystemAccess().getCalibrationList());
		QProgressDialog progress(tr("Downloading files..."), tr("Abort"), 0, fileList.size());
		progress.setWindowModality(Qt::WindowModal);

		// download files
		bool res = downloadCalib(fileList, cacheDir, &progress);
		if (!res) {
			// the download was cancelled
			return;
		}

		// preprocess images
		progress.setLabelText(tr("Preprocessing images..."));
		progress.setValue(0);
		Lyli::Calibration::Calibrator calibrator;
		res = preprocess(calibrator, cacheDir, &progress);
		if (!res) {
			// the preprocess was cancelled
			return;
		}

		// run calibration
		progress.setLabelText(tr("Calibrating images..."));
		progress.setValue(0);
		::Lyli::Calibration::CalibrationData calibration = calibrate(calibrator, &progress);

		// store the result
		LyliConfig::storeCalibrationData(calibration);
	}
}

bool CameraCalibrator::downloadCalib(Lyli::Filesystem::ImageList &fileList, const QDir &dir, QProgressDialog *progress) {
	std::stringstream ss;
	std::ofstream ofs;
	int done = 0;
	for (const auto &image : fileList) {
		progress->setValue(++done);

		ss << image->getName() << ".TXT";
		QString txtpath = dir.filePath(QString::fromStdString(ss.str()));
		// skip if the file exists
		if (!QFileInfo(txtpath).exists()) {
			ofs.open(txtpath.toLocal8Bit().constData(), std::ofstream::out | std::ofstream::binary);
			image->getImageMetadata(ofs);
			ofs.flush();
			ofs.close();
		}
		ss.str("");
		ss.clear();

		ss << image->getName() << ".RAW";
		QString rawpath = dir.filePath(QString::fromStdString(ss.str()));
		if (!QFileInfo(rawpath).exists()) {
			ofs.open(rawpath.toLocal8Bit().constData(), std::ofstream::out | std::ofstream::binary);
			image->getImageData(ofs);
			ofs.flush();
			ofs.close();
		}
		ss.str("");
		ss.clear();

		if(progress->wasCanceled()) {
			// TODO: delete on cancel?
			return false;
		}
	}

	return true;
}

bool CameraCalibrator::preprocess(Lyli::Calibration::Calibrator& calibrator, const QDir &dir, QProgressDialog *progress) {
	// read all files
	QStringList filters = {"*.RAW"};
	QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::Readable);
	std::vector<std::string> files;
	for (auto & file : fileList) {
		QString absolutePath = file.absoluteFilePath();
		absolutePath.truncate(absolutePath.lastIndexOf('.'));
		files.push_back(absolutePath.toLocal8Bit().constData());
	}

	// calibrate
	Lyli::Calibration::LensDetector lensDetector(std::make_unique<Lyli::Calibration::FFTPreprocessor>());
	std::atomic_int done(0);
	try {
		tbb::parallel_for_each(
			std::begin(files),
			std::end(files),
			[&calibrator,&lensDetector, &progress, &done](const auto &filebase) {

			progress->setValue(++done);
			std::stringstream ss;

			// read metadata
			ss << filebase << ".TXT";
			std::fstream finmeta(ss.str(), std::fstream::in | std::fstream::binary);
			if (!finmeta.good()) {
				// missing metadata, skip
				return;
			}
			ss.str("");
			ss.clear();
			Lyli::Image::Metadata metadata(finmeta);

			// read image
			ss << filebase << ".RAW";
			std::fstream fin(ss.str(), std::fstream::in | std::fstream::binary);
			ss.str("");
			ss.clear();
			Lyli::Image::RawImage rawimg(fin, 3280, 3280);

			// detect the lenses
			Lyli::Calibration::PointGrid pointGrid = lensDetector.detect(rawimg.getData());
			if (pointGrid.isEmpty()) {
				// image is too flat, skip
				return;
			}

			// add grid with the lenses to the calibrator
			calibrator.addGrid(pointGrid, metadata);

			// TODO: handle cancel
		});
	} catch (Lyli::Calibration::CameraDiffersException& e) {
		// TODO handle error
		return false;
	}

	return true;
}

::Lyli::Calibration::CalibrationData CameraCalibrator::calibrate(Lyli::Calibration::Calibrator &calibrator, QProgressDialog *progress) {
	// change to busy indicator
	progress->setRange(0, 0);
	progress->setValue(0);

	// calibrate
	Lyli::Calibration::CalibrationData calibrationResult = calibrator.calibrate();

	return calibrationResult;
}
