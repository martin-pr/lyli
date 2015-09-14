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

#include "file.h"

#include <cassert>
#include <memory>
#include <utility>

#include "camera.h"

namespace Lyli {

class File::Impl {
public:
	Impl(Camera *camera, const std::string path);
	Impl(const Impl& other);

	Camera *m_camera;
	std::string m_path;
};

File::Impl::Impl(Camera *camera, const std::string path) : m_camera(camera), m_path(path) {

}

File::Impl::Impl(const File::Impl& other) : m_camera(other.m_camera), m_path(other.m_path) {

}

File::File() : pimpl(nullptr) {

}

File::File(Camera *camera, const std::string &path) : pimpl(new Impl(camera, path)) {

}

File::File(File&& other) noexcept : pimpl(std::move(other.pimpl)) {

}

File::~File() {

}

File::File(const File& other) : pimpl(new Impl(*other.pimpl)) {

}

File& File::operator=(const File& other) {
	if (this == &other) {
		return *this;
	}

	File tmp(other);
	std::swap(pimpl, tmp.pimpl);
	return *this;
}

File& File::operator=(File&& other) noexcept {
	if (this == &other) {
		return *this;
	}

	std::swap(pimpl, other.pimpl);
	return *this;
}

std::string File::getName() {
	std::size_t pos = pimpl->m_path.find_last_of('\\');
	if(pos != std::string::npos) {
		return pimpl->m_path.substr(pos);
	}
	return pimpl->m_path;
}

void File::download(std::ostream& os) {
	pimpl->m_camera->getFile(os, pimpl->m_path);
}

}

template class std::shared_ptr<Lyli::File>;
