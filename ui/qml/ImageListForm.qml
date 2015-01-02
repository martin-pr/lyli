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

import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

GridView {
    id: imageGrid
    Layout.fillHeight: true
    Layout.fillWidth: true
    cellWidth: 148
    cellHeight: 188
    model: imageGridModel

    delegate: Item {
        width: parent.cellWidth - 4
        height: parent.cellHeight - 4

        Column {
            Rectangle {
                width: 144
                height: 184
                color: model.selected ? "lightblue" : "white"
                border.color: "black"
                border.width: 1

                Image {
                    id: image1
                    x: 8
                    y: 8
                    width: 128
                    height: 128
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    asynchronous: true
                    source: "image://ThumbnailProvider/" + model.id
                }

                Label {
                    id: date
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: 142
                    text: Qt.formatDateTime(model.datetime, "d MMM yyyy")
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    id: time
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: 162
                    text: Qt.formatDateTime(model.datetime, "hh:mm:ss")
                    horizontalAlignment: Text.AlignHCenter
                }

                /* The action for selecting the view */
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (mouse.button == Qt.LeftButton) {
                            if (mouse.modifiers == Qt.ControlModifier) {
                                imageGridModel.setAdditiveSelection()
                            }
                            if (mouse.modifiers == Qt.ShiftModifier) {
                                imageGridModel.setRangeSelection()
                            }
                        }
                        model.selected = !model.selected
                    }
                }
            }
        }
    }
}
