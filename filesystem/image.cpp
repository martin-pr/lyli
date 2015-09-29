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

#include "image.h"

#include <camera.h>

namespace Lyli {
namespace Filesystem {

class Image::Impl {
public:
	Camera *m_camera;
	std::string m_fullpath; // the full path (including basename)
	std::string m_basename;

	Impl(Camera *camera, const std::string &path, const std::string basename);
	Impl(const Impl& other);
};

Image::Impl::Impl(Camera *camera, const std::string &path, const std::string basename)
: m_camera(camera), m_fullpath(path + basename), m_basename(basename) {

}

Image::Impl::Impl(const Impl& other)
: m_camera(other.m_camera), m_fullpath(other.m_fullpath), m_basename(other.m_basename) {

}

Image::Image() {

}

Image::Image(Camera *camera, const std::string &path, const std::string basename) : pimpl(new Impl(camera, path, basename)) {

}

Image::Image(const Image& other) : pimpl(new Impl(*other.pimpl)) {

}

Image::Image(Image&& other) noexcept : pimpl(std::move(other.pimpl)) {

}

Image::~Image() {

}

Image& Image::operator=(const Image& other) {
	if(this == &other) {
		return *this;
	}
	pimpl = std::make_unique<Impl>(*other.pimpl);
	return *this;
}

Image& Image::operator=(Image&& other) noexcept {
	if(this == &other) {
		return *this;
	}
	pimpl = std::move(other.pimpl);
	return *this;
}

std::string Image::getName() {
	return pimpl->m_basename;
}

void Image::getImageMetadata(std::ostream &os) const {
	pimpl->m_camera->getFile(os, pimpl->m_fullpath + ".TXT");
}

void Image::getImageData(std::ostream &os) const {
	pimpl->m_camera->getFile(os, pimpl->m_fullpath + ".RAW");
}

Lyli::Camera * Image::getCamera() const {
	return pimpl->m_camera;
}

std::string &Image::getFullPath() const {
	return pimpl->m_fullpath;
}


}
}
