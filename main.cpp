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
#include <unistd.h>

#include <opencv2/opencv.hpp>

#include <libusbpp/context.h>

#include <camera.h>
#include <calibration/calibrator.h>
#include <filesystem/filesystemaccess.h>
#include <filesystem/photo.h>
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
			// skip the file
			continue;
		}
		std::string filebase(file.substr(0, file.size() - 4));
		files.push_back(filebase);
	}
	closedir(dir);

	// sort the files (it's more user-friendly)
	std::sort(files.begin(), files.end());

	// calibrate
	std::stringstream ss;
	for (const auto &filebase : files) {
		ss << filebase << ".RAW";
		std::cout << "reading image: " << ss.str() << std::endl;
		std::fstream fin(ss.str(), std::fstream::in | std::fstream::binary);
		ss.str("");
		ss.clear();

		std::cout << "calibrating image..." << std::endl;

		Lyli::Calibration::Calibrator calibrator;
		Lyli::Image::RawImage rawimg(fin, 3280, 3280);
		/*calibrator.addImage(rawimg.getData());
		calibrator.calibrate();*/

		ss << filebase << "-color.png";
		std::cout << "writing calibration grid: " << ss.str() << std::endl;
		cv::imwrite(ss.str(), rawimg.getData());
		ss.str("");
		ss.clear();
	}
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
	while ((c = getopt(argc, argv, "ild:t:c:f:")) != -1) {
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
	while ((c = getopt(argc, argv, "ild:t:c:f:")) != -1) {
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
				calibrate(optarg);
				return 0;
			case 'f':
				downloadFile(camera, optarg);
				return 0;
			default:
				showHelp();
				return 1;
		}
	}
}
