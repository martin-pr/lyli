/*
 * This file is part of Lyli-Qt, a GUI to control Lytro camera
 * Copyright (C) 2015  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QString>
#include <QStringList>
#include <QObjectList>

#include "imagelistmodel.h"

#include <context.h>
#include <camera.h>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	Usbpp::Context context;
	Lyli::CameraList cameras(Lyli::getCameras(context));

	// get list of cameras
	QStringList cameraList;
	for (Lyli::Camera &camera : cameras) {
		Lyli::CameraInformation info(camera.getCameraInformation());
		QString name = QString(info.vendor.c_str()).trimmed() + " "
				+ QString(info.product.c_str()).trimmed() + " "
				+ QString(info.revision.c_str()).trimmed();
		cameraList.append(name);
	}

	// get list of files
	ImageListModel imageList(cameras);

	// preselect the first camera
	if (cameras.size() > 0) {
		imageList.changeCamera(0);
	}

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("cameraListModel", QVariant::fromValue(cameraList));
	engine.rootContext()->setContextProperty("imageGridModel", &imageList);
	engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

	return app.exec();
}
