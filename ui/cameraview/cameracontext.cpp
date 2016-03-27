/*
 * This file is part of Lyli, an application to control Lytro camera
 * Copyright (C) 2016  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
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
 * 
 */

#include "cameracontext.h"

Context::Context() : m_current(-1) {

}

Context::~Context() {

}

void Context::updateCameraList() {
	m_cameraList = m_context.getCameras();
	emit cameraListChanged();
}

Lyli::CameraList::size_type Context::getCameraCount() const {
	return m_cameraList.size();
}

Lyli::Camera* Context::getCamera(Lyli::CameraList::size_type index) {
	if (index >= m_cameraList.size()) {
		return nullptr;
	}
	return &(m_cameraList[index]);
}

void Context::changeCurrentCamera(Lyli::CameraList::size_type index) {
	if (m_current < 0 || (static_cast<Lyli::CameraList::size_type>(m_current) != index && index < m_cameraList.size())) {
		m_current = index;
		emit cameraChanged(&(m_cameraList[m_current]));
	}
}

