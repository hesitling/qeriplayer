import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Page {
    id: root

    readonly property var detailVm: mainVm ? mainVm.neteasePlaylistDetail : null

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 88
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

                Rectangle {
                    Layout.preferredWidth: 56
                    Layout.preferredHeight: 56
                    Layout.alignment: Qt.AlignVCenter
                    color: Qt.rgba(1, 1, 1, 0.05)
                    radius: 6

                    Image {
                        id: coverImage
                        anchors.fill: parent
                        anchors.margins: 2
                        source: detailVm ? detailVm.headerCoverUrl : ""
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        visible: status === Image.Ready
                    }

                    Label {
                        anchors.centerIn: parent
                        visible: coverImage.status !== Image.Ready
                        text: "📚"
                        font.pixelSize: 20
                        opacity: 0.4
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        text: detailVm ? detailVm.headerName : "Playlist"
                        font.pixelSize: 22
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Label {
                        text: detailVm ? (detailVm.headerTrackCount + (detailVm.headerTrackCount === 1 ? " track" : " tracks")) : ""
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
                    text: "Save to Local"
                    enabled: detailVm && detailVm.songs.count > 0
                    onClicked: detailVm.saveToLocal()
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            BusyIndicator {
                anchors.centerIn: parent
                visible: detailVm && detailVm.isLoading
                running: visible
            }

            Column {
                anchors.centerIn: parent
                spacing: 12
                visible: detailVm && !detailVm.isLoading && detailVm.hasError

                Label {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: detailVm ? detailVm.error.message : "Failed to load"
                    color: Material.hintTextColor
                    horizontalAlignment: Text.AlignHCenter
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Retry"
                    onClicked: detailVm.retry()
                }
            }

            ListView {
                anchors.fill: parent
                clip: true
                model: detailVm ? detailVm.songs : null
                visible: detailVm && !detailVm.isLoading && !detailVm.hasError && detailVm.songs.count > 0
                delegate: SongDelegate {
                    required property int index
                    onDoubleClicked: detailVm.playSong(index)
                }
                ScrollBar.vertical: ScrollBar {}
            }
        }
    }
}
