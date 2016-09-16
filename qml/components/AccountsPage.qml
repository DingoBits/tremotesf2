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

Page {
    id: accountsPage

    allowedOrientations: defaultAllowedOrientations

    SilicaListView {
        id: listView

        property alias selectionModel: selectionModel

        anchors {
            fill: parent
            bottomMargin: selectionPanel.visibleSize
        }
        clip: true

        header: PageHeader {
            title: qsTranslate("tremotesf", "Accounts")
        }

        model: BaseProxyModel {
            id: proxyModel

            sortRole: AccountsModel.NameRole
            sourceModel: AccountsModel {
                id: accountsModel
            }

            Component.onCompleted: sort()
        }
        delegate: ListItem {
            id: delegate

            property var modelData: model
            property bool selected

            function remove() {
                Accounts.removeAccount(modelData.name)
                accountsModel.removeAccountAtRow(modelData.index)
            }

            highlighted: down || menuOpen || selected

            menu: Component {
                ContextMenu {
                    MenuItem {
                        text: qsTranslate("tremotesf", "Edit...")
                        onClicked: pageStack.push("AccountEditDialog.qml", {"accountsModel": accountsModel,
                                                                            "modelData": modelData})
                    }

                    MenuItem {
                        text: qsTranslate("tremotesf", "Remove")
                        onClicked: remorseAction(qsTranslate("tremotesf", "Removing %1").arg(modelData.name), remove)
                    }
                }
            }
            showMenuOnPressAndHold: !selectionPanel.openPanel

            onClicked: {
                if (selectionPanel.openPanel) {
                    selectionModel.select(modelData.index)
                } else {
                    pageStack.push("AccountEditDialog.qml", {"accountsModel": accountsModel,
                                                             "modelData": modelData})
                }
            }

            onPressAndHold: {
                if (selectionPanel.openPanel) {
                    selectionModel.select(modelData.index)
                }
            }

            Component.onCompleted: selected = selectionModel.isSelected(modelData.index)

            Connections {
                target: selectionModel
                onSelectionChanged: selected = selectionModel.isSelected(modelData.index)
            }

            Switch {
                id: currentSwitch

                enabled: !selectionPanel.openPanel

                anchors.verticalCenter: parent.verticalCenter
                automaticCheck: false
                checked: modelData.current

                highlighted: down || delegate.highlighted

                onClicked: {
                    Accounts.setCurrentAccount(modelData.name)
                    modelData.current = true
                }
            }

            Label {
                anchors {
                    left: currentSwitch.right
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                color: highlighted ? Theme.highlightColor : Theme.primaryColor
                text: modelData.name
                truncationMode: TruncationMode.Fade
            }
        }

        SelectionModel {
            id: selectionModel
            model: proxyModel
        }

        PullDownMenu {
            MenuItem {
                text: qsTranslate("tremotesf", "Select")
                onClicked: selectionPanel.show()
            }

            MenuItem {
                text: qsTranslate("tremotesf", "Add...")
                onClicked: pageStack.push("AccountEditDialog.qml", {"accountsModel": accountsModel})
            }
        }

        ViewPlaceholder {
            enabled: !listView.count
            text: qsTranslate("tremotesf", "No accounts")
        }
    }

    RemorsePopup {
        id: remorsePopup
    }

    SelectionPanel {
        id: selectionPanel
        selectionModel: listView.selectionModel
        text: qsTranslate("tremotesf", "%n account(s) selected", String(), selectionModel.selectedIndexesCount)

        PushUpMenu {
            MenuItem {
                enabled: selectionModel.hasSelection
                text: qsTranslate("tremotesf", "Remove")
                onClicked: remorsePopup.execute(qsTranslate("tremotesf", "Removing %n account(s)", String(), selectionModel.selectedIndexesCount),
                                                function() {
                                                    while (selectionModel.hasSelection) {
                                                        accountsModel.removeAccountAtIndex(proxyModel.sourceIndex(selectionModel.firstSelectedIndex))
                                                    }
                                                })
            }
        }
    }
}
