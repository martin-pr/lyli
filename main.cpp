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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <tbb/parallel_for_each.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <libusbpp/context.h>

#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>

#include <camera.h>
#include <calibration/calibrator.h>
#include <calibration/fftpreprocessor.h>
#include <calibration/lensdetector.h>
#include <calibration/pointgrid.h>
#include <filesystem/filesystemaccess.h>
#include <filesystem/photo.h>
#include <image/lightfieldimage.h>
#include <image/metadata.h>
#include <image/rawimage.h>

void showHelp() {
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "\t-i\t get basic camera information" << std::endl;
	std::cout << "\t-l\t list files" << std::endl;
	std::cout << "\t-d id\t download image with the specified id" << std::endl;
	std::cout << "\t     \t The image is downloaded to the current directory" << std::endl;
	std::cout << "\t-t dir\t download calibration images to a specified directory" << std::endl;
	std::cout << "\t-c dir\t calibrate using files from a specified directory" << std::endl;
	std::cout << "\t      \t The output is stored in file \"calibration.json\"" << std::endl;
	std::cout << "\t-p dir\t process images in the selected directory." << std::endl;
	std::cout << "\t      \t The option requires a file \"calibration.json\" to exist" << std::endl;
	std::cout << "\t      \t in the selected directory." << std::endl;
	std::cout << "\t-f path\t download a file specified by a full path, potentialy dangerous" << std::endl;
	std::cout << "\t     \t Requires knowledge of the camera file structure." << std::endl;
}

/**
 * Prepare the selected camera and return a pointer to the camera to use.
 */
Lyli::Camera* prepareCamera(Lyli::CameraList &cameraList, std::size_t idx) {
	std::cout << "waiting until the camera is ready" << std::endl;
	cameraList[idx].waitReady();
	std::cout << "camera ready" << std::endl;
	return &cameraList[idx];
}

void getCameraInformation(Lyli::Camera *camera) {
	Lyli::CameraInformation info(camera->getCameraInformation());

	std::cout << "Vendor: " << info.vendor << std::endl;
	std::cout << "Product: " << info.product << std::endl;
	std::cout << "Revision: " << info.revision << std::endl;
}

void listFiles(Lyli::Camera *camera) {
	Lyli::Filesystem::PhotoList fileList(camera->getFilesystemAccess().getPictureList());

	std::cout << std::setw(3) <<"id" << std::setw(28) << "date" << std::endl;
	std::size_t i(0);
	for (auto file : fileList) {
		std::cout << std::setw(3) << i++;

		// time
		std::time_t time(file->getTime());
		std::tm *tm = std::localtime(&time);
		char buf[29];
		std::strftime(buf, 28, "%c", tm);
		buf[28] = '\0';
		std::cout << std::setw(28) << buf;

		std::cout << std::endl;
	}
}

void downloadImage(Lyli::Camera *camera, int id) {
	Lyli::Filesystem::PhotoList fileList(camera->getFilesystemAccess().getPictureList());
	::Lyli::Filesystem::Photo *photo = fileList[id].get();

	std::stringstream ss;
	std::ofstream ofs;

	ss << photo->getName() << ".TXT";
	ofs.open(ss.str(), std::ofstream::out | std::ofstream::binary);
	photo->getImageMetadata(ofs);
	ofs.flush();
	ofs.close();
	ss.str("");
	ss.clear();

	ss << photo->getName() << ".128";
	ofs.open(ss.str(), std::ofstream::out | std::ofstream::binary);
	photo->getImageThumbnail(ofs);
	ofs.flush();
	ofs.close();
	ss.str("");
	ss.clear();

	ss << photo->getName() << ".RAW";
	ofs.open(ss.str(), std::ofstream::out | std::ofstream::binary);
	photo->getImageData(ofs);
	ofs.flush();
	ofs.close();
	ss.str("");
	ss.clear();
}

void downloadCalib(Lyli::Camera *camera, const std::string &path) {
	Lyli::Filesystem::ImageList fileList(camera->getFilesystemAccess().getCalibrationList());

	std::stringstream ss;
	std::ofstream ofs;
	if (chdir(path.c_str()) != 0) {
		std::perror("failed to change directory");
		return;
	}
	for (const auto &image : fileList) {
		std::cout << "downloading file: " << image->getName() << std::endl;

		ss << image->getName() << ".TXT";
		ofs.open(ss.str(), std::ofstream::out | std::ofstream::binary);
		image->getImageMetadata(ofs);
		ofs.flush();
		ofs.close();
		ss.str("");
		ss.clear();

		ss << image->getName() << ".RAW";
		ofs.open(ss.str(), std::ofstream::out | std::ofstream::binary);
		image->getImageData(ofs);
		ofs.flush();
		ofs.close();
		ss.str("");
		ss.clear();
	}
}

void calibrate(const std::string& path, const std::string& out) {
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
			// skip the file
			continue;
		}
		std::string filebase(file.substr(0, file.size() - 4));
		files.push_back(filebase);
	}
	closedir(dir);

	// calibrate
	Lyli::Calibration::Calibrator calibrator;
	Lyli::Calibration::LensDetector lensDetector(std::make_unique<Lyli::Calibration::FFTPreprocessor>());
	try {
		tbb::parallel_for_each(
			std::begin(files),
			std::end(files),
			[&calibrator,&lensDetector](const auto &filebase) {
			std::cout << filebase << " reading image..." << std::endl;
			std::stringstream ss;

			// read metadata
			ss << filebase << ".TXT";
			std::fstream finmeta(ss.str(), std::fstream::in | std::fstream::binary);
			if (!finmeta.good()) {
				std::cout << filebase << " missing metadata, skipping" << std::endl;
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
			std::cout << filebase << " processing image..." << std::endl;

			Lyli::Calibration::PointGrid pointGrid = lensDetector.detect(rawimg.getData());
			if (pointGrid.isEmpty()) {
				std::cout << filebase << " image is too flat, skipping" << std::endl;
				return;
			}

			// add grid with the lenses to the calibrator
			calibrator.addGrid(pointGrid, metadata);
		});
	} catch (Lyli::Calibration::CameraDiffersException& e) {
		std::cerr << e.what() << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// CALIBRATE!
	std::cout << "calibrating images..." << std::endl;
	Lyli::Calibration::CalibrationData calibrationResult = calibrator.calibrate();
	std::cout << "DONE" << std::endl;

	// store the results
	Json::Value json = calibrationResult.serialize();
	std::ofstream fout(out, std::fstream::out | std::fstream::trunc | std::fstream::binary);
	Json::StyledStreamWriter styledWriter;
	styledWriter.write(fout, json);
	fout.close();
}

void process(const std::string& path, const std::string& in) {
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
			// skip the file
			continue;
		}
		std::string filebase(file.substr(0, file.size() - 4));
		files.push_back(filebase);
	}
	closedir(dir);

	// read calibration data
	std::fstream fin(in, std::fstream::in | std::fstream::binary);
	Json::CharReaderBuilder readerbuilder;
	Json::Value root;
	Json::parseFromStream(readerbuilder, fin, &root, 0);
	Lyli::Calibration::CalibrationData calibration;
	calibration.deserialize(root);

	tbb::parallel_for_each(files, [&calibration](const auto &filebase) {
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

		// straighten etc.
		Lyli::Image::LightfieldImage lightfieldimg(rawimg, metadata, calibration);
		ss << filebase << "-flat.png";
		cv::imwrite(ss.str(), lightfieldimg.getData());
		ss.str("");
		ss.clear();
	});
}

void downloadFile(Lyli::Camera *camera, const std::string &path) {
	std::size_t sepPos = path.find_last_of("/\\");
	std::string outputFile(sepPos != std::string::npos ? path.substr(sepPos + 1) : path);
	std::ofstream ofs(outputFile.c_str(), std::ofstream::out | std::ofstream::binary);

	camera->getFile(ofs, path);
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		showHelp();
		return 0;
	}

	Usbpp::Context context;
	Lyli::CameraList cameras(Lyli::getCameras(context));
	Lyli::Camera *camera(0);

	// first prepare camera if we are calling a function requiring camera to be operating
	int c;
	while ((c = getopt(argc, argv, "ild:t:c:f:p:")) != -1) {
		switch (c) {
			case 'i':
			case 'l':
			case 'd':
			case 't':
			case 'f':
				if (cameras.size() == 0) {
					std::cerr << "No cameras found" << std::endl;
					return 1;
				}
				camera = prepareCamera(cameras, 0);
				break;
		}
	}
	optind = 1;

	// process the options
	while ((c = getopt(argc, argv, "ild:t:c:f:p:")) != -1) {
		switch (c) {
			case 'i':
				getCameraInformation(camera);
				break;
			case 'l':
				listFiles(camera);
				return 0;
			case 'd':
				downloadImage(camera, atoi(optarg));
				return 0;
			case 't':
				downloadCalib(camera, optarg);
				return 0;
			case 'c':
				calibrate(optarg, "calibration.json");
				return 0;
			case 'f':
				downloadFile(camera, optarg);
				return 0;
			case 'p':
				process(optarg, "calibration.json");
				return 0;
			default:
				showHelp();
				return 1;
		}
	}
}
