/*
 * Tremotesf
 * Copyright (C) 2015-2016 Alexey Rochev <equeim@gmail.com>
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

import QtQuick 2.2
import Sailfish.Silica 1.0

import harbour.tremotesf 1.0

Dialog {
    id: addTorrentFileDialog

    property string filePath

    allowedOrientations: defaultAllowedOrientations
    canAccept: filesModel.loaded && downloadDirectoryItem.text

    onAccepted: rpc.addTorrentFile(parser.fileData,
                                   downloadDirectoryItem.text,
                                   filesModel.wantedFiles,
                                   filesModel.unwantedFiles,
                                   filesModel.highPriorityFiles,
                                   filesModel.normalPriorityFiles,
                                   filesModel.lowPriorityFiles,
                                   1 - priorityComboBox.currentIndex,
                                   startSwitch.checked)

    TorrentFileParser {
        id: parser
        filePath: addTorrentFileDialog.filePath
        onDone: {
            if (error == TorrentFileParser.NoError) {
                filesModel.load(parser)
            }
        }
    }

    LocalTorrentFilesModel {
        id: filesModel
    }

    BusyIndicator {
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
        running: {
            if (!filePath) {
                return false
            }

            if (parser.error === TorrentFileParser.NoError) {
                return !parser.loaded || !filesModel.loaded
            }

            return false
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        ViewPlaceholder {
            enabled: parser.error != TorrentFileParser.NoError
            text: parser.errorString
        }

        Column {
            id: column

            width: parent.width
            visible: filesModel.loaded

            DialogHeader {
                title: qsTranslate("tremotesf", "Add Torrent File")
            }

            TextField {
                width: parent.width
                label: qsTranslate("tremotesf", "Torrent file")
                text: filePath
                readOnly: true
            }

            FileSelectionItem {
                id: downloadDirectoryItem

                label: qsTranslate("tremotesf", "Download directory")
                selectionButtonEnabled: rpc.local
                showFiles: false
                Component.onCompleted: text = rpc.serverSettings.downloadDirectory
            }

            Item {
                width: parent.width
                height: filesButton.height + Theme.paddingLarge

                Button {
                    id: filesButton

                    anchors.centerIn: parent
                    width: Theme.buttonWidthLarge
                    text: qsTranslate("tremotesf", "Files")

                    onClicked: pageStack.push("LocalTorrentFilesPage.qml", {"filesModel": filesModel})
                }
            }

            ComboBox {
                id: priorityComboBox

                label: qsTranslate("tremotesf", "Torrent priority")
                menu: ContextMenu {
                    MenuItem {
                        text: qsTranslate("tremotesf", "High")
                    }
                    MenuItem {
                        text: qsTranslate("tremotesf", "Normal")
                    }
                    MenuItem {
                        text: qsTranslate("tremotesf", "Low")
                    }
                }
                currentIndex: 1
            }

            TextSwitch {
                id: startSwitch
                text: qsTranslate("tremotesf", "Start downloading after adding")
                Component.onCompleted: checked = rpc.serverSettings.startAddedTorrents
            }
        }

        VerticalScrollDecorator { }
    }
}
