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

#ifndef LYLI_FILELIST_H_
#define LYLI_FILELIST_H_

#include <array>
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

namespace Lyli {

class Camera;

typedef std::array<uint8_t, 20> Sha1Array;

class FileListEntry {
public:
	FileListEntry();
	FileListEntry(Camera *camera, const std::string &dirBase, int dirId, const std::string &fileBase, int fileId, const Sha1Array &sha1, const std::time_t &time);
	~FileListEntry();
	
	FileListEntry(const FileListEntry &other);
	FileListEntry &operator=(const FileListEntry &other);
	
	FileListEntry(FileListEntry &&other) noexcept;
	FileListEntry &operator=(FileListEntry &&other) noexcept;
	
	/**
	 * Time the picture was taken.
	 */
	std::time_t getTime() const;
	
	void getImageMetadata(std::ostream &os) const;
	void getImageThumbnail(std::ostream &os) const;
	void getImageData(std::ostream &os) const;
	
private:
	class Impl;
	Impl *pimpl;
};

typedef std::vector<FileListEntry> FileList;

} // Lyli

#endif
