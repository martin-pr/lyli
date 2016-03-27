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

#ifndef CAMERACALIBRATOR_H
#define CAMERACALIBRATOR_H

#include <QtCore/QObject>

namespace Lyli {
class Camera;
}

/**
 * Checks whether a camera has been calibrated and opens calibration wizard if needed
 */
class CameraCalibrator : public QObject {
Q_OBJECT
public:
	CameraCalibrator(QObject *parent = nullptr);
	~CameraCalibrator();
	CameraCalibrator(const CameraCalibrator& other) = delete;
	CameraCalibrator& operator=(const CameraCalibrator& other) = delete;

public slots:
	void onCameraChanged(Lyli::Camera* camera);
};

#endif // CAMERACALIBRATOR_H
