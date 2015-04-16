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

#include "filelistparser.h"

#include <ctime>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>

#include "camera.h"

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
	Lyli::FileListEntry parseLine(Lyli::Camera *camera, const uint8_t *line) {
		std::string dirBase = reinterpret_cast<const char*>(line);
		std::string fileBase = reinterpret_cast<const char*>(line + 8);
		int dirId = * reinterpret_cast<const uint32_t*>(line + 16);
		int fileId = * reinterpret_cast<const uint32_t*>(line + 20);
		
		// sha1 (excluding the "sha1-")
		Lyli::Sha1Array sha1;
		const char *psha1(reinterpret_cast<const char*>(line + 48 + 5));
		for (int i(0); i < 20; ++i) {
			int tmp;
			std::sscanf(psha1+ 2*i, "%02x", &tmp);
			sha1[i] = tmp;
		}
		
		// parse time
		const char *ptime(reinterpret_cast<const char*>(line + 96));
		std::tm timeStruct = {};
		std::sscanf(ptime, "%04d-%02d-%02d", &(timeStruct.tm_year), &(timeStruct.tm_mon), &(timeStruct.tm_mday));
		std::sscanf(ptime + 11, "%02d:%02d:%02d", &(timeStruct.tm_hour), &(timeStruct.tm_min), &(timeStruct.tm_sec));
		// fix the std::tm weirdness...
		timeStruct.tm_mon -= 1;
		timeStruct.tm_year -= 1900;
		std::time_t time = std::mktime(&timeStruct) -  timezone;
		
		return Lyli::FileListEntry(camera, dirBase, dirId, fileBase, fileId, sha1, time);
	}
}

namespace Lyli {

FileList parseFileList(Camera *camera, const Usbpp::ByteBuffer& buffer)
{
	FileList fileList;
	// length of the line
	std::ptrdiff_t linelen(* reinterpret_cast<const uint32_t*>(buffer.data() + 4));
	// position in the data (begins at the first entry)
	std::ptrdiff_t pos((*reinterpret_cast<const uint32_t*>(buffer.data() + 8)) * 8 + 12);
	
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
