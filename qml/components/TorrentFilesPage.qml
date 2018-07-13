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

        model: DelegateModel {
            id: delegateModel

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

            delegate: ListItem {
                id: delegate

                property var modelData: model
                property bool selected

                highlighted: down || menuOpen || selected

                menu: Component {
                    ContextMenu {
                        MenuItem {
                            id: openMenuItem
                            visible: torrentPropertiesPage.torrentIsLocal && modelData.wantedState !== TorrentFilesModelEntry.Unwanted
                            text: qsTranslate("tremotesf", "Open")
                            onClicked: Qt.openUrlExternally(filesModel.localFilePath(filesProxyModel.sourceIndex(delegateModel.modelIndex(modelData.index))))
                        }

                        Separator {
                            visible: openMenuItem.visible
                            width: parent.width
                            color: Theme.secondaryColor
                        }

                        MenuItem {
                            visible: modelData.wantedState !== TorrentFilesModelEntry.Wanted
                            text: qsTranslate("tremotesf", "Download", "File menu item, verb")
                            onClicked: filesModel.setFileWanted(filesProxyModel.sourceIndex(delegateModel.modelIndex(modelData.index)), true)
                        }

                        MenuItem {
                            visible: modelData.wantedState !== TorrentFilesModelEntry.Unwanted
                            text: qsTranslate("tremotesf", "Not Download")
                            onClicked: filesModel.setFileWanted(filesProxyModel.sourceIndex(delegateModel.modelIndex(modelData.index)), false)
                        }

                        Separator {
                            width: parent.width
                            color: Theme.secondaryColor
                        }

                        MenuLabel {
                            text: qsTranslate("tremotesf", "Priority")
                        }

                        MenuItem {
                            font.bold: modelData.priority === TorrentFilesModelEntry.HighPriority
                            //: Priority
                            text: qsTranslate("tremotesf", "High")
                            onClicked: {
                                if (modelData.priority !== TorrentFilesModelEntry.HighPriority) {
                                    filesModel.setFilePriority(filesProxyModel.sourceIndex(delegateModel.modelIndex(modelData.index)), TorrentFilesModelEntry.HighPriority)
                                }
                            }
                        }

                        MenuItem {
                            font.bold: modelData.priority === TorrentFilesModelEntry.NormalPriority
                            //: Priority
                            text: qsTranslate("tremotesf", "Normal")
                            onClicked: {
                                if (modelData.priority !== TorrentFilesModelEntry.NormalPriority) {
                                    filesModel.setFilePriority(filesProxyModel.sourceIndex(delegateModel.modelIndex(modelData.index)), TorrentFilesModelEntry.NormalPriority)
                                }
                            }
                        }

                        MenuItem {
                            font.bold: modelData.priority === TorrentFilesModelEntry.LowPriority
                            //: Priority
                            text: qsTranslate("tremotesf", "Low")
                            onClicked: {
                                if (modelData.priority !== TorrentFilesModelEntry.LowPriority) {
                                    filesModel.setFilePriority(filesProxyModel.sourceIndex(delegateModel.modelIndex(modelData.index)), TorrentFilesModelEntry.LowPriority)
                                }
                            }
                        }

                        MenuItem {
                            visible: modelData.priority === TorrentFilesModelEntry.MixedPriority
                            font.bold: true
                            //: Priority
                            text: qsTranslate("tremotesf", "Mixed")
                        }

                        Separator {
                            width: parent.width
                            color: Theme.secondaryColor
                        }

                        MenuItem {
                            text: qsTranslate("tremotesf", "Rename")
                            onClicked: pageStack.push(renameDialogComponent)
                            Component.onCompleted: visible = rpc.serverSettings.canRenameFiles
                        }
                    }
                }
                showMenuOnPressAndHold: !selectionPanel.openPanel

                onClicked: {
                    if (selectionPanel.openPanel) {
                        selectionModel.select(delegateModel.modelIndex(modelData.index))
                    } else if (modelData.isDirectory) {
                        delegateModel.rootIndex = delegateModel.modelIndex(modelData.index)
                    }
                }

                onPressAndHold: {
                    if (selectionPanel.openPanel) {
                        selectionModel.select(delegateModel.modelIndex(modelData.index))
                    }
                }

                Component.onCompleted: selected = selectionModel.isSelected(delegateModel.modelIndex(modelData.index))

                Connections {
                    target: selectionModel
                    onSelectionChanged: selected = selectionModel.isSelected(delegateModel.modelIndex(modelData.index))
                }

                Component {
                    id: renameDialogComponent
                    FileRenameDialog { }
                }

                Image {
                    id: icon
                    anchors {
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                    source: {
                        var iconSource
                        if (modelData.isDirectory) {
                            iconSource = "image://theme/icon-m-folder"
                        } else {
                            iconSource = "image://theme/icon-m-other"
                        }

                        if (highlighted) {
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

                Column {
                    anchors {
                        left: icon.right
                        leftMargin: Theme.paddingMedium
                        right: wantedSwitch.left
                        verticalCenter: parent.verticalCenter
                    }

                    Label {
                        width: parent.width
                        color: highlighted ? Theme.highlightColor : Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        text: modelData.name
                        truncationMode: TruncationMode.Fade
                    }

                    Item {
                        width: parent.width
                        height: Theme.paddingSmall

                        Rectangle {
                            id: progressRectangle

                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width * modelData.progress
                            height: parent.height / 2

                            color: Theme.highlightColor
                            opacity: 0.6
                        }

                        Rectangle {
                            anchors {
                                left: progressRectangle.right
                                right: parent.right
                                verticalCenter: parent.verticalCenter
                            }
                            height: progressRectangle.height

                            color: Theme.highlightBackgroundColor
                            opacity: Theme.highlightBackgroundOpacity
                        }
                    }

                    Label {
                        width: parent.width
                        color: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                        font.pixelSize: Theme.fontSizeExtraSmall

                        //: e.g. 100 MiB of 200 MiB (50%)
                        text: qsTranslate("tremotesf", "%1 of %2 (%3)")
                        .arg(Utils.formatByteSize(modelData.completedSize))
                        .arg(Utils.formatByteSize(modelData.size))
                        .arg(Utils.formatProgress(modelData.progress))

                        truncationMode: TruncationMode.Fade
                    }
                }

                Switch {
                    id: wantedSwitch

                    anchors {
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }
                    highlighted: down || delegate.highlighted

                    enabled: !selectionPanel.openPanel

                    automaticCheck: false
                    checked: modelData.wantedState !== TorrentFilesModelEntry.Unwanted
                    opacity: (modelData.wantedState === TorrentFilesModelEntry.MixedWanted) ? 0.6 : 1

                    onClicked: filesModel.setFileWanted(filesProxyModel.sourceIndex(delegateModel.modelIndex(modelData.index)), !checked)
                }
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

    SelectionPanel {
        id: selectionPanel
        selectionModel: listView.selectionModel
        parentIndex: delegateModel.rootIndex
        text: qsTranslate("tremotesf", "%n files selected", String(), selectionModel.selectedIndexesCount)

        PushUpMenu {
            MenuItem {
                enabled: selectionModel.hasSelection
                text: qsTranslate("tremotesf", "Download", "File menu item, verb")
                onClicked: {
                    filesModel.setFilesWanted(filesProxyModel.sourceIndexes(selectionModel.selectedIndexes), true)
                    selectionPanel.hide()
                }
            }

            MenuItem {
                enabled: selectionModel.hasSelection
                text: qsTranslate("tremotesf", "Not Download")
                onClicked: {
                    filesModel.setFilesWanted(filesProxyModel.sourceIndexes(selectionModel.selectedIndexes), false)
                    selectionPanel.hide()
                }
            }

            MenuLabel {
                text: qsTranslate("tremotesf", "Priority")
            }

            MenuItem {
                enabled: selectionModel.hasSelection
                //: Priority
                text: qsTranslate("tremotesf", "High")
                onClicked: {
                    filesModel.setFilesPriority(filesProxyModel.sourceIndexes(selectionModel.selectedIndexes), TorrentFilesModelEntry.HighPriority)
                    selectionPanel.hide()
                }
            }

            MenuItem {
                enabled: selectionModel.hasSelection
                //: Priority
                text: qsTranslate("tremotesf", "Normal")
                onClicked: {
                    filesModel.setFilesPriority(filesProxyModel.sourceIndexes(selectionModel.selectedIndexes), TorrentFilesModelEntry.NormalPriority)
                    selectionPanel.hide()
                }
            }

            MenuItem {
                enabled: selectionModel.hasSelection
                //: Priority
                text: qsTranslate("tremotesf", "Low")
                onClicked: {
                    filesModel.setFilesPriority(filesProxyModel.sourceIndexes(selectionModel.selectedIndexes), TorrentFilesModelEntry.LowPriority)
                    selectionPanel.hide()
                }
            }
        }

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
