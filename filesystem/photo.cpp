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

#include "photo.h"

#include <camera.h>

#include <iomanip>
#include <sstream>

namespace {

std::string constructPath(const std::string &dirBase, int dirId) {
	std::stringstream ss;
	ss << "I:\\DCIM\\";
	ss << dirId << dirBase << "\\";
	return ss.str();
}

std::string constructBaseName(const std::string &fileBase, int fileId) {
	std::stringstream ss;
	ss << fileBase << std::setfill ('0') << std::setw (4) << fileId;
	return ss.str();
}

}

namespace Lyli {
namespace Filesystem {

class Photo::Impl {
public:
	Sha1Array m_sha1;
	std::time_t m_time;

	Impl(const Sha1Array &sha1, const std::time_t &time);
	Impl(const Impl &other);
};

Photo::Impl::Impl(const Sha1Array &sha1, const std::time_t &time) : m_sha1(sha1), m_time(time) {

}

Photo::Impl::Impl(const Impl &other) : m_sha1(other.m_sha1), m_time(other.m_time) {

}

Photo::Photo() {

}

Photo::Photo(Camera *camera, const std::string &dirBase, int dirId, const std::string &fileBase, int fileId, const Sha1Array &sha1, const std::time_t &time )
	: Image(camera, constructPath(dirBase, dirId), constructBaseName(fileBase, fileId)), pimpl(new Impl(sha1, time)) {

}

Photo::Photo(const Photo& other) : Image(other), pimpl(new Impl(*other.pimpl)) {

}

Photo::~Photo() {

}

Photo& Photo::operator=(const Photo& other) {
	Image::operator=(other);
	pimpl = std::make_unique<Impl>(*other.pimpl);
	return *this;
}

std::time_t Photo::getTime() const {
	return pimpl->m_time;
}

void Photo::getImageThumbnail ( std::ostream &os ) const {
	getCamera()->getFile(os, getFullPath() + ".128");
}

}
}
