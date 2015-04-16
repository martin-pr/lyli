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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unistd.h>

#include "usbpp/context.h"
#include "camera.h"

void showHelp() {
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "\t-i\t get basic camera information" << std::endl;
	std::cout << "\t-l\t list files" << std::endl;
	std::cout << "\t-d id\t download image with the specified id" << std::endl;
	std::cout << "\t     \t The image is downloaded to the current directory" << std::endl;
}

void waitForCamera(Lyli::Camera &camera) {
	std::cout << "waiting until the camera is ready" << std::endl;
	camera.waitReady();
	std::cout << "camera ready" << std::endl;
}

void getCameraInformation(Lyli::Camera &camera) {
	Lyli::CameraInformation info(camera.getCameraInformation());
	
	std::cout << "Vendor: " << info.vendor << std::endl;
	std::cout << "Product: " << info.product << std::endl;
	std::cout << "Revision: " << info.revision << std::endl;
}

void listFiles(Lyli::Camera &camera) {
	Lyli::FileList fileList(camera.getPictureList());
	
	std::cout << std::setw(3) <<"id" << std::setw(28) << "date" << std::endl;
	std::size_t i(0);
	for (Lyli::FileListEntry file : fileList) {
		std::cout << std::setw(3) << i++;
		
		// time
		std::time_t time(file.getTime());
		std::tm *tm = std::localtime(&time);
		char buf[29];
		std::strftime(buf, 28, "%c", tm);
		buf[28] = '\0';
		std::cout << std::setw(28) << buf;
		
		std::cout << std::endl;
	}
}

void downloadImage(Lyli::Camera &camera, int id) {
	Lyli::FileList fileList(camera.getPictureList());
	
	char outputFile[50];
	std::ofstream ofs;
	
	std::snprintf(outputFile, 50, "%04d.TXT", id);
	ofs.open(outputFile, std::ofstream::out | std::ofstream::binary);
	fileList[id].getImageMetadata(ofs);
	ofs.flush();
	ofs.close();
	
	std::snprintf(outputFile, 50, "%04d.128", id);
	ofs.open(outputFile, std::ofstream::out | std::ofstream::binary);
	fileList[id].getImageThumbnail(ofs);
	ofs.flush();
	ofs.close();
	
	std::snprintf(outputFile, 50, "%04d.RAW", id);
	ofs.open(outputFile, std::ofstream::out | std::ofstream::binary);
	fileList[id].getImageData(ofs);
	ofs.flush();
	ofs.close();
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		showHelp();
		return 0;
	}
	
	Usbpp::Context context;
	Lyli::CameraList cameras(Lyli::getCameras(context));
	
	if (cameras.size() == 0) {
		std::cerr << "No cameras found" << std::endl;
		return 1;
	}
	
	Lyli::Camera &camera(cameras[0]);
	
	int c;
	while ((c = getopt(argc, argv, "ild:")) != -1) {
		switch (c) {
			case 'i':
				waitForCamera(camera);
				getCameraInformation(camera);
				break;
			case 'l':
				waitForCamera(camera);
				listFiles(camera);
				return 0;
			case 'd':
				waitForCamera(camera);
				downloadImage(camera, atoi(optarg));
				return 0;
			default:
				showHelp();
				return 1;
		}
	}
}