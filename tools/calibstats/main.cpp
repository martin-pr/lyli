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

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <utility>

#include <tbb/parallel_for_each.h>

#include <calibration/calibrator.h>
#include <calibration/fftpreprocessor.h>
#include <calibration/lensdetector.h>
#include <calibration/pointgrid.h>
#include <image/metadata.h>
#include <image/rawimage.h>

void showHelp() {
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "\tcalibstats path/to/calibration/files" << std::endl;
}

void calibrate(const std::string &path) {
	std::vector<std::string> files;

	if (chdir(path.c_str()) != 0) {
		std::perror("failed to change directory");
		return;
	}

	// read all files
	DIR *dir = opendir(".");
	if (dir == nullptr) {
		std::perror("failed to change directory");
		return;
	}
	dirent *ent;
	const std::string ext(".RAW");
	while ((ent = readdir(dir)) != nullptr) {
		std::string file(ent->d_name);
		if (file.size() < ext.size()
		    || ! std::equal(ext.rbegin(), ext.rend(), file.rbegin())) {
			// skip the files without the .RAW extension
			continue;
		}
		std::string filebase(file.substr(0, file.size() - 4));
		files.push_back(filebase);
	}
	closedir(dir);
	//files.push_back("MOD_0054");

	// sort the files (it's more user-friendly)
	std::sort(files.begin(), files.end());

	// add images to the calibrator
	Lyli::Calibration::Calibrator calibrator;
	Lyli::Calibration::LensDetector lensDetector(std::make_unique<Lyli::Calibration::FFTPreprocessor>());
	tbb::parallel_for_each(files, [&calibrator,&lensDetector](const auto &filebase){
		std::cout << filebase << " reading image..." << std::endl;
		std::stringstream ss;

		// read image
		ss << filebase << ".RAW";
		std::fstream fin(ss.str(), std::fstream::in | std::fstream::binary);
		ss.str("");
		ss.clear();
		Lyli::Image::RawImage rawimg(fin, 3280, 3280);

		// read metadata
		ss << filebase << ".TXT";
		std::fstream finmeta(ss.str(), std::fstream::in | std::fstream::binary);
		ss.str("");
		ss.clear();
		Lyli::Image::Metadata metadata(finmeta);
		// detect the lenses
		std::cout << filebase << " processing image..." << std::endl;

		Lyli::Calibration::PointGrid pointGrid = lensDetector.detect(rawimg.getData());
		if (pointGrid.isEmpty()) {
			std::cout << filebase << " image is too flat, skipping" << std::endl;
			return;
		}

		// add grid with the lenses to the calibrator
		calibrator.addGrid(pointGrid, metadata);
	});

	// CALIBRATE!
	std::cout << "calibrating images..." << std::endl;
	auto calibrationResult = calibrator.calibrate();
	std::cout << "DONE" << std::endl;

	// prepare the output of the results
	std::ofstream ofs("XXX.csv", std::ofstream::out | std::ofstream::binary);

	// print header
	ofs << "Zoomstep,"
		<< "Focusstep,"
		// translation
		<< "tx,"
		<< "ty,"
		// rotation
		<< "rotation,"
		// camera matrix
		<< "fx,"
		<< "fy,"
		<< "cx,"
		<< "cy,"
		// distortion coefficients
		<< "k1,"
		<< "k2,"
		<< "p1,"
		<< "p2,"
		<< "k3,"
		<< std::endl;
	// store the results
	for (const auto& res : calibrationResult.getLens()) {
		ofs << res.first.getZoomStep() << ","
			<< res.first.getFocusStep() << ","
			// translation
			<< calibrationResult.getArray().getTranslation()[0] << "," // tx
			<< calibrationResult.getArray().getTranslation()[1] << "," // ty
			// rotation
			<< calibrationResult.getArray().getRotation() << "," // rotation
			// camera matrix
			<< res.second.getCameraMatrix().at<double>(0,0) << "," // fx
			<< res.second.getCameraMatrix().at<double>(1,1) << "," // fy
			<< res.second.getCameraMatrix().at<double>(0,2) << "," // cx
			<< res.second.getCameraMatrix().at<double>(1,2) << "," // cy
			// distortion coefficients
			<< res.second.getDistCoeffs().at<double>(0) << "," // k1
			<< res.second.getDistCoeffs().at<double>(1) << "," // k2
			<< res.second.getDistCoeffs().at<double>(2) << "," // p1
			<< res.second.getDistCoeffs().at<double>(3) << "," // p2
			<< res.second.getDistCoeffs().at<double>(4) << "," // k3
			<< std::endl;
	}

	ofs.close();
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		showHelp();
		return 0;
	}

	calibrate(argv[1]);
	return 0;
}

