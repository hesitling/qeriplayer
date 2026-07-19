import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Dialog {
    id: aboutDialog
    title: "About QeriPlayer"
    modal: true
    anchors.centerIn: parent
    width: 360
    standardButtons: Dialog.Ok

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        Label {
            text: "🎵 QeriPlayer Qt"
            font.pixelSize: 20
            font.weight: Font.Bold
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: "Version " + Qt.application.version
            Layout.alignment: Qt.AlignHCenter
            color: Material.hintTextColor
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Material.dividerColor
        }

        Label {
            text: "Built with:"
            font.weight: Font.DemiBold
        }

        Label {
            text: "• Qt " + Qt.version
            color: Material.foreground
        }

        Label {
            text: "• QCoro 0.10+"
            color: Material.foreground
        }

        Label {
            text: "• C++20"
            color: Material.foreground
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Material.dividerColor
        }

        Label {
            text: "© 2026 QeriPlayer"
            opacity: 0.6
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
