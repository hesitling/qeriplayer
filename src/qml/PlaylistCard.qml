import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

ItemDelegate {
    id: root

    property string title: ""
    property string subtitle: ""
    property string coverUrl: ""

    width: ListView.view ? ListView.view.width : 0
    height: 72

    background: Rectangle {
        color: root.highlighted ? Qt.rgba(1, 1, 1, 0.05) : "transparent"
    }

    contentItem: RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 12
        spacing: 12

        Rectangle {
            Layout.preferredWidth: 52
            Layout.preferredHeight: 52
            Layout.alignment: Qt.AlignVCenter
            color: Qt.rgba(1, 1, 1, 0.05)
            radius: 6

            Image {
                id: coverImage
                anchors.fill: parent
                anchors.margins: 2
                source: root.coverUrl
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
            Layout.alignment: Qt.AlignVCenter
            spacing: 4

            Label {
                Layout.fillWidth: true
                text: root.title
                font.pixelSize: 14
                font.weight: Font.Medium
                elide: Text.ElideRight
                color: Material.foreground
            }

            Label {
                Layout.fillWidth: true
                text: root.subtitle
                font.pixelSize: 11
                color: Material.hintTextColor
                elide: Text.ElideRight
            }
        }
    }
}
