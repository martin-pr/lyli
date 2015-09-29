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

#include "filesystemaccess.h"

#include <camera.h>
#include <filesystem/image.h>
#include <filesystem/photo.h>

#include "libusbpp/buffer.h"

#include <ctime>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>

namespace {
/**
 * Parse an entry for one file from the file list.
 *
 * The entry (line) has 124 bytes. It is separated into several
 * parts, each containing some information about the file.
 *
 * The structure is following:
 *   8B - PHOTO - 8 B
 *   8B - IMG - 8 B
 *   4B - d
 *   4B - image counter
 *   24B - ???
 *   48B - sha1
 *   24B - time (10B - date, 1B - T, 12B - time, 1B - Z)
 *   4B - padding?
 */
Lyli::Filesystem::PhotoPtr parseLine(Lyli::Camera *camera, const uint8_t *line) {
	std::string dirBase = reinterpret_cast<const char*>(line);
	std::string fileBase = reinterpret_cast<const char*>(line + 8);
	int dirId = * reinterpret_cast<const uint32_t*>(line + 16);
	int fileId = * reinterpret_cast<const uint32_t*>(line + 20);

	// sha1 (excluding the "sha1-")
	Lyli::Filesystem::Sha1Array sha1;
	const char *psha1(reinterpret_cast<const char*>(line + 48 + 5));
	for (int i(0); i < 20; ++i) {
		unsigned int tmp;
		std::sscanf(psha1+ 2*i, "%02x", &tmp);
		sha1[i] = tmp;
	}

	// parse time
	const char *ptime(reinterpret_cast<const char*>(line + 96));
	std::tm timeStruct;
	std::sscanf(ptime, "%04d-%02d-%02d", &(timeStruct.tm_year), &(timeStruct.tm_mon), &(timeStruct.tm_mday));
	std::sscanf(ptime + 11, "%02d:%02d:%02d", &(timeStruct.tm_hour), &(timeStruct.tm_min), &(timeStruct.tm_sec));
	// fix the std::tm weirdness...
	timeStruct.tm_mon -= 1;
	timeStruct.tm_year -= 1900;
	std::time_t time = std::mktime(&timeStruct) -  timezone;

	return Lyli::Filesystem::PhotoPtr(new Lyli::Filesystem::Photo(camera, dirBase, dirId, fileBase, fileId, sha1, time));
}

Lyli::Filesystem::PhotoList parseFileList(Lyli::Camera *camera, const Usbpp::ByteBuffer& buffer) {
	Lyli::Filesystem::PhotoList fileList;
	// length of the line
	std::ptrdiff_t linelen(* reinterpret_cast<const uint32_t*>(buffer.data() + 4));
	// position in the data (begins at the first entry)
	std::size_t pos((*reinterpret_cast<const uint32_t*>(buffer.data() + 8)) * 8 + 12);

	// each file has an entry long exactly 124 bytes
	while (pos + linelen <= buffer.size()) {
		const uint8_t *line(buffer.data() + pos);
		// now parse the line
		fileList.push_back(parseLine(camera, line));
		pos += linelen;
	}

	return fileList;
}
}

namespace Lyli {
namespace Filesystem {

class FilesystemAccess::Impl {
public:
	Camera *m_camera;

	Impl(Camera *camera);
	Impl(const Impl &other);
};

FilesystemAccess::Impl::Impl(Camera *camera) : m_camera(camera) {

}

FilesystemAccess::Impl::Impl(const Impl &other) : m_camera(other.m_camera) {

}

FilesystemAccess::FilesystemAccess(Camera *camera) : pimpl(new Impl(camera)) {

}

FilesystemAccess::FilesystemAccess(const FilesystemAccess &other) : pimpl(new Impl(*other.pimpl)) {

}

FilesystemAccess::~FilesystemAccess() {

}

void FilesystemAccess::getFirmware(std::ostream &os) {
	pimpl->m_camera->getFile(os, "A:\\FIRMWARE.TXT");
}

void FilesystemAccess::getVCM(std::ostream &os) {
	pimpl->m_camera->getFile(os, "A:\\VCM.TXT");
}

PhotoList FilesystemAccess::getPictureList() {
	Usbpp::ByteBuffer buf_ = pimpl->m_camera->getPictureList();
	return parseFileList(pimpl->m_camera, buf_);
}

/*ImageList FilesystemAccess::getCalibrationList() {

}*/

}
}
