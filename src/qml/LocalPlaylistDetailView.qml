import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Page {
    id: root

    readonly property var detailVm: mainVm ? mainVm.localPlaylistDetail : null

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 72
            color: Material.dialogColor

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 12

                ToolButton {
                    text: "←"
                    onClicked: mainVm.navigateTo(2)
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        text: detailVm ? detailVm.playlistName : "Playlist"
                        font.pixelSize: 22
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Label {
                        text: detailVm ? (detailVm.songs.count + (detailVm.songs.count === 1 ? " song" : " songs")) : ""
                        color: Material.hintTextColor
                        elide: Text.ElideRight
                    }
                }

                Button {
                    text: "Play All"
                    enabled: detailVm && detailVm.songs.count > 0
                    onClicked: detailVm.playAll()
                }

                Button {
                    text: "Rename"
                    enabled: detailVm !== null
                    onClicked: {
                        renameField.text = detailVm ? detailVm.playlistName : ""
                        renameDialog.open()
                    }
                }

                Button {
                    text: "Delete"
                    enabled: detailVm !== null
                    onClicked: deleteDialog.open()
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Label {
                anchors.centerIn: parent
                visible: detailVm && !detailVm.hasError && detailVm.songs.count === 0
                text: "This playlist is empty"
                font.pixelSize: 16
                color: Material.hintTextColor
            }

            Label {
                anchors.centerIn: parent
                visible: detailVm && detailVm.hasError
                text: detailVm ? detailVm.error.message : "Failed to load playlist"
                font.pixelSize: 16
                color: Material.hintTextColor
            }

            ListView {
                anchors.fill: parent
                clip: true
                model: detailVm ? detailVm.songs : null
                visible: detailVm && !detailVm.hasError && detailVm.songs.count > 0
                delegate: SongDelegate {
                    required property int index
                    onDoubleClicked: detailVm.playSong(index)
                }
                ScrollBar.vertical: ScrollBar {}
            }
        }
    }

    Dialog {
        id: renameDialog
        title: "Rename Playlist"
        modal: true
        anchors.centerIn: parent
        width: 360
        standardButtons: Dialog.NoButton

        ColumnLayout {
            width: parent.width
            spacing: 16

            TextField {
                id: renameField
                Layout.fillWidth: true
                placeholderText: "Playlist name"
                onAccepted: renameButton.clicked()
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: "Cancel"
                    onClicked: renameDialog.close()
                }

                Button {
                    id: renameButton
                    text: "Save"
                    enabled: detailVm && renameField.text.trim().length > 0
                    onClicked: {
                        detailVm.rename(renameField.text.trim())
                        renameDialog.close()
                    }
                }
            }
        }
    }

    Dialog {
        id: deleteDialog
        title: "Delete Playlist"
        modal: true
        anchors.centerIn: parent
        width: 360
        standardButtons: Dialog.NoButton

        ColumnLayout {
            width: parent.width
            spacing: 16

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: "Delete this playlist permanently?"
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: "Cancel"
                    onClicked: deleteDialog.close()
                }

                Button {
                    text: "Delete"
                    enabled: detailVm !== null
                    onClicked: {
                        detailVm.deletePlaylist()
                        deleteDialog.close()
                        mainVm.navigateTo(2)
                    }
                }
            }
        }
    }
}
