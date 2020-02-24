/*
 * Tremotesf
 * Copyright (C) 2015-2018 Alexey Rochev <equeim@gmail.com>
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
import QtQml.Models 2.1
import Sailfish.Silica 1.0

import harbour.tremotesf 1.0

Page {
    id: torrentFilesPage

    property var torrent: torrentPropertiesPage.torrent

    allowedOrientations: defaultAllowedOrientations

    SilicaListView {
        id: listView

        property alias selectionModel: selectionModel

        anchors {
            fill: parent
            bottomMargin: selectionPanel.visibleSize
        }
        clip: true

        header: Column {
            width: listView.width

            onHeightChanged: {
                if (listView.contentY < 0) {
                    listView.positionViewAtBeginning()
                }
            }

            TorrentRemovedHeader {
                torrent: torrentFilesPage.torrent
            }

            PageHeader {
                title: qsTranslate("tremotesf", "Files")
            }

            BackgroundItem {
                id: parentDirectoryItem

                visible: delegateModel.rootIndex !== delegateModel.parentModelIndex()
                onClicked: {
                    if (!selectionPanel.openPanel) {
                        delegateModel.rootIndex = delegateModel.parentModelIndex()
                    }
                }

                Image {
                    id: parentDirectoryIcon
                    anchors {
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                    source: {
                        var iconSource = "image://theme/icon-m-folder"
                        if (parentDirectoryItem.highlighted) {
                            iconSource += "?"
                            iconSource += Theme.highlightColor
                        }
                        return iconSource
                    }
                    sourceSize {
                        width: Theme.iconSizeSmallPlus
                        height: Theme.iconSizeSmallPlus
                    }
                }

                Label {
                    anchors {
                        left: parentDirectoryIcon.right
                        leftMargin: Theme.paddingMedium
                        verticalCenter: parent.verticalCenter
                    }
                    color: parentDirectoryItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    text: ".."
                }
            }
        }

        model: BaseTorrentFilesDelegateModel {
            id: delegateModel

            addingTorrent: false

            model: TorrentFilesProxyModel {
                id: filesProxyModel
                sourceModel: TorrentFilesModel {
                    id: filesModel
                    torrent: torrentFilesPage.torrent
                    rpc: rootWindow.rpc
                }
                sortRole: TorrentFilesModel.NameRole
                Component.onCompleted: sort()
            }
        }

        SelectionModel {
            id: selectionModel
            model: filesProxyModel
        }

        PullDownMenu {
            MenuItem {
                visible: filesModel.loaded && torrentPropertiesPage.torrentIsLocal && filesModel.isWanted(filesProxyModel.sourceIndex(delegateModel.rootIndex))
                text: qsTranslate("tremotesf", "Open")
                onClicked: Qt.openUrlExternally(filesModel.localFilePath(filesProxyModel.sourceIndex(delegateModel.rootIndex)))
            }

            MenuItem {
                enabled: filesModel.loaded
                text: qsTranslate("tremotesf", "Select")
                onClicked: selectionPanel.show()
            }
        }

        BusyIndicator {
            anchors.centerIn: parent
            size: BusyIndicatorSize.Large
            running: filesModel.loading
        }

        ViewPlaceholder {
            enabled: !filesModel.loading && !listView.count
            text: qsTranslate("tremotesf", "No files")
        }

        VerticalScrollDecorator { }
    }

    TorrentFilesSelectionPanel {
        id: selectionPanel

        Connections {
            target: rpc
            onConnectedChanged: {
                if (!rpc.connected) {
                    selectionPanel.hide()
                }
            }
        }
    }
}
