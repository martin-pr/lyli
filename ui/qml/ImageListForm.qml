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
    cellWidth: 110
    cellHeight: 130
    model: imageGridModel

    delegate: Item {
        x: 5
        width: 110
        height: 130

        Column {
            Rectangle {
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

                /* Constructing the view */
                width: 110
                height: 130
                color: model.selected ? "blue" : "white"
                border.color: "black"
                border.width: 1
                anchors.horizontalCenter: parent.horizontalCenter

                Rectangle {
                    y: 5
                    width: 100
                    height: 100
                    color: "grey"
                    anchors.horizontalCenter: parent.horizontalCenter

                    Column {
                        Text {
                            id: date
                            text: Qt.formatDateTime(model.datetime, "d MMM yyyy")
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            id: time
                            text: Qt.formatDateTime(model.datetime, "hh:mm:ss")
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }

                Text {
                    text: model.id
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    font.bold: true
                }
            }
        }
    }
}
