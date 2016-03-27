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
#ifndef CONTEXT_H
#define CONTEXT_H

#include <memory>

#include <QtCore/QObject>

#include <camera.h>
#include <context.h>
#include <libusbpp/context.h>

class Context : public QObject {
	Q_OBJECT
public:
	Context();
	~Context();

	void updateCameraList();

	/**
	 * Get the count of cameras.
	 */
	Lyli::CameraList::size_type getCameraCount() const;
	/**
	 * Get camera at the given index
	 */
	Lyli::Camera* getCamera(Lyli::CameraList::size_type index);

public slots:
	/**
	 * Change the currently selected camera
	 */
	void changeCurrentCamera(Lyli::CameraList::size_type index);

signals:
	/**
	 * Camera has changed
	 * \param camera pointer to a new camera
	 */
	void cameraChanged(Lyli::Camera* camera);

	/**
	 * Camera list has changed.
	 */
	void cameraListChanged();

private:
	Lyli::Context m_context;
	Lyli::CameraList m_cameraList;
	int m_current;
};

#endif // CONTEXT_H
