import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Dialog {
    id: loginDialog
    objectName: "cookieDialog"
    title: "Import NetEase Cookie"
    modal: true
    anchors.centerIn: parent
    width: 520
    standardButtons: Dialog.NoButton

    readonly property bool importInFlight: settingsVm.isImportingNeteaseCookie

    onOpened: {
        settingsVm.clearError()
        cookieField.text = ""
    }

    onClosed: settingsVm.clearError()

    Connections {
        target: settingsVm
        function onIsNeteaseLoggedInChanged() {
            if (settingsVm.isNeteaseLoggedIn) {
                loginDialog.close()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        TextArea {
            id: cookieField
            objectName: "cookieField"
            Layout.fillWidth: true
            Layout.preferredHeight: 180
            placeholderText: "MUSIC_U=...; __csrf=..."
            wrapMode: TextEdit.WrapAnywhere
            selectByMouse: true
        }

        Label {
            visible: settingsVm.hasError
            text: settingsVm.error.message
            color: Material.color(Material.Red)
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Item {
                Layout.fillWidth: true
            }

            Button {
                objectName: "cancelCookieButton"
                text: "Cancel"
                enabled: !loginDialog.importInFlight
                onClicked: loginDialog.close()
            }

            Button {
                id: importButton
                objectName: "importCookieButton"
                text: loginDialog.importInFlight ? "Importing..." : "Import"
                enabled: cookieField.text.trim().length > 0 && !loginDialog.importInFlight
                onClicked: {
                    settingsVm.clearError()
                    settingsVm.importNeteaseCookie(cookieField.text.trim())
                }
            }
        }
    }
}
