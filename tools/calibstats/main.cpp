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

#include <calibration/calibrator.h>
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

	std::vector<Lyli::Calibration::CalibrationData> calibData;


	std::stringstream ss;
	std::ofstream ofs("XXX.csv", std::ofstream::out | std::ofstream::binary);

	// print header
	ofs << "FILE,"
		<< "Infinitylambda,"
		<< "Focallength,"
		<< "Zoomstep,"
		<< "Focusstep,"
		<< "Fnumber,"
		<< "Zoomstepperoffset,"
		<< "Focusstepperoffset,"
		<< "Exitpupiloffset,"
		// translation
		<< "tx,"
		<< "ty,"
		// rotation
		<< "r00,"
		<< "r10,"
		<< "r01,"
		<< "r11,"
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

	// calibrate and store stats
	for (const auto &filebase : files) {
		ss << filebase << ".RAW";
		std::cout << "reading image: " << ss.str() << std::endl;
		std::fstream fin(ss.str(), std::fstream::in | std::fstream::binary);
		ss.str("");
		ss.clear();

		std::cout << "calibrating image..." << std::endl;

		Lyli::Calibration::Calibrator calibrator;
		Lyli::Image::RawImage rawimg(fin, 3280, 3280);
		calibrator.addImage(rawimg.getData());
		calibrator.calibrate();
		calibData.push_back(calibrator.getCalibrationData());

		// read metadata
		ss << filebase << ".TXT";
		std::cout << "reading image: " << ss.str() << std::endl;
		std::fstream finmeta(ss.str(), std::fstream::in | std::fstream::binary);
		ss.str("");
		ss.clear();
		Lyli::Image::Metadata metadata(finmeta);
		Lyli::Image::Metadata::Devices::Lens lensdata(metadata.getDevices().getLens());
		Lyli::Calibration::CalibrationData calibdata(calibrator.getCalibrationData());

		ofs << filebase << ","
			<< lensdata.getInfinitylambda() << ","
			<< lensdata.getFocallength() << ","
			<< lensdata.getZoomstep() << ","
			<< lensdata.getFocusstep() << ","
			<< lensdata.getFnumber() << ","
			<< lensdata.getZoomstepperoffset() << ","
			<< lensdata.getFocusstepperoffset() << ","
			<< lensdata.getExitpupiloffset().getZ() << ","
			// translation
			<< calibdata.getTranslation().at<double>(0,2) << "," // tx
			<< calibdata.getTranslation().at<double>(1,2) << "," // ty
			// rotation
			<< calibdata.getRotation().at<double>(0,0) << "," // r00
			<< calibdata.getRotation().at<double>(0,1) << "," // r10
			<< calibdata.getRotation().at<double>(1,0) << "," // r01
			<< calibdata.getRotation().at<double>(1,1) << "," // r11
			// camera matrix
			<< calibdata.getCameraMatrix().at<double>(0,0) << "," // fx
			<< calibdata.getCameraMatrix().at<double>(1,1) << "," // fy
			<< calibdata.getCameraMatrix().at<double>(0,2) << "," // cx
			<< calibdata.getCameraMatrix().at<double>(1,2) << "," // cy
			// distortion coefficients
			<< calibdata.getDistCoeffs().at<double>(0) << "," // k1
			<< calibdata.getDistCoeffs().at<double>(1) << "," // k2
			<< calibdata.getDistCoeffs().at<double>(2) << "," // p1
			<< calibdata.getDistCoeffs().at<double>(3) << "," // p2
			<< calibdata.getDistCoeffs().at<double>(4) << "," // k3
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

