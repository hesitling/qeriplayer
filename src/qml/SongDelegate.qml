import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

ItemDelegate {
    id: root

    width: ListView.view ? ListView.view.width : 0
    height: 64

    // Playing highlight
    property bool isCurrentSong: playerVm && playerVm.currentSong
                                 && model.id === playerVm.currentSong.id

    // Platform display name helper
    function platformDisplayName(platform) {
        switch (platform) {
        case 1:
            return "NetEase"
        case 2:
            return "Bilibili"
        case 3:
            return "YouTube"
        case 4:
            return "QQ Music"
        default:
            return ""
        }
    }

    // Duration formatting helper
    function formatDuration(ms) {
        var totalSec = Math.floor(ms / 1000)
        var min = Math.floor(totalSec / 60)
        var sec = totalSec % 60
        return min + ":" + (sec < 10 ? "0" : "") + sec
    }

    background: Rectangle {
        color: root.isCurrentSong ? Qt.rgba(Material.accentColor.r, Material.accentColor.g,
                                            Material.accentColor.b, 0.12) : (root.highlighted ? Qt.rgba(1, 1, 1, 0.05) : "transparent")
    }

    contentItem: RowLayout {
        spacing: 12
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 12

        // Cover art
        Rectangle {
            Layout.preferredWidth: 48
            Layout.preferredHeight: 48
            Layout.alignment: Qt.AlignVCenter
            color: Qt.rgba(1, 1, 1, 0.05)
            radius: 4

            Image {
                id: coverImage
                anchors.fill: parent
                anchors.margins: 2
                source: model.coverUrl || ""
                fillMode: Image.PreserveAspectCrop
                visible: status === Image.Ready
                asynchronous: true
            }

            Label {
                anchors.centerIn: parent
                visible: coverImage.status !== Image.Ready
                text: "\uD83C\uDFB5" // 🎵
                font.pixelSize: 20
                opacity: 0.4
            }
        }

        // Song info
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 2

            Label {
                Layout.fillWidth: true
                text: model.name || ""
                font.pixelSize: 13
                font.weight: Font.Medium
                elide: Text.ElideRight
                opacity: root.isCurrentSong ? 1.0 : 0.87
                color: root.isCurrentSong ? Material.accentColor : Material.foreground
            }

            Label {
                Layout.fillWidth: true
                text: {
                    var parts = []
                    if (model.artist)
                        parts.push(model.artist)
                    if (model.album)
                        parts.push(model.album)
                    return parts.join(" · ") || ""
                }
                font.pixelSize: 11
                color: Material.hintTextColor
                elide: Text.ElideRight
            }
        }

        // Platform badge
        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            visible: platformDisplayName(model.platform) !== ""
            radius: 4
            height: 20
            implicitWidth: badgeLabel.implicitWidth + 12
            color: Qt.rgba(1, 1, 1, 0.08)

            Label {
                id: badgeLabel
                anchors.centerIn: parent
                text: platformDisplayName(model.platform)
                font.pixelSize: 10
                color: Material.hintTextColor
            }
        }

        // Duration
        Label {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 40
            horizontalAlignment: Text.AlignRight
            text: formatDuration(model.durationMs || 0)
            font.pixelSize: 12
            color: Material.hintTextColor
        }
    }
}
