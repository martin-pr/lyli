/*
 * This file is part of Litro-Qt, a GUI to control Lytro camera
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

import QtQuick 2.0
import QtQuick.Layouts 1.1

ListView {
    id: cameraList
    Layout.fillHeight: true
    width: parent.width / 4
    spacing: 10
    model: cameraListModel

    delegate: Item {
        x: 5
        width: 80
        height: 40

        Column {
            id: cameraColumn

            Text {
                text: model.modelData
                font.bold: true
            }

            Column {
                id: cameraOptsColumn
                x: 8

                Text {
                    text: qsTr("Camera Information")
                }

                Text {
                    text: qsTr("Photos")
                }
            }
        }
    }
}
