import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Qt.labs.platform as Platform
import QeriPlayer 1.0

Page {
    id: root

    readonly property var audioQualityOptions: [
        { name: "Low", value: AudioQuality.Low },
        { name: "Standard", value: AudioQuality.Standard },
        { name: "High", value: AudioQuality.High },
        { name: "Lossless", value: AudioQuality.Lossless }
    ]

    function audioQualityIndex(quality) {
        for (var index = 0; index < audioQualityOptions.length; ++index) {
            if (audioQualityOptions[index].value === quality)
                return index
        }
        return -1
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            color: Material.dialogColor

            Label {
                anchors.fill: parent
                anchors.leftMargin: 16
                text: "Settings"
                font.pixelSize: 24
                font.weight: Font.DemiBold
                verticalAlignment: Text.AlignVCenter
            }
        }

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentHeight: settingsColumn.implicitHeight
            clip: true
            flickableDirection: Flickable.VerticalFlick

            ScrollBar.vertical: ScrollBar {}

            Column {
                id: settingsColumn
                width: parent.width
                spacing: 24
                padding: 16

                GroupBox {
                    width: parent.width - 32
                    title: "General"

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 16

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Label {
                                text: "Theme"
                                Layout.preferredWidth: 120
                            }

                            ComboBox {
                                Layout.fillWidth: true
                                model: ["Light", "Dark"]
                                currentIndex: settingsVm.theme === "dark" ? 1 : 0
                                onActivated: index => {
                                    settingsVm.setTheme(index === 1 ? "dark" : "light")
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Label {
                                text: "Audio Quality"
                                Layout.preferredWidth: 120
                            }

                            ComboBox {
                                objectName: "audioQualityCombo"
                                Layout.fillWidth: true
                                model: root.audioQualityOptions
                                textRole: "name"
                                valueRole: "value"
                                currentIndex: root.audioQualityIndex(settingsVm.audioQuality)
                                onActivated: settingsVm.setAudioQuality(currentValue)
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Label {
                                text: "Download Path"
                                Layout.preferredWidth: 120
                            }

                            TextField {
                                Layout.fillWidth: true
                                text: settingsVm.downloadPath
                                placeholderText: "Select download folder..."
                                onEditingFinished: {
                                    settingsVm.setDownloadPath(text)
                                }
                            }

                            Button {
                                text: "Browse"
                                onClicked: folderDialog.open()
                            }
                        }
                    }
                }

                GroupBox {
                    width: parent.width - 32
                    title: "NetEase Account"

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 12

                        Label {
                            objectName: "neteaseStatusLabel"
                            text: settingsVm.isNeteaseLoggedIn ? "Logged in as " + settingsVm.neteaseUsername : "Not logged in"
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Button {
                                id: accountActionButton
                                objectName: "accountActionButton"
                                text: settingsVm.isNeteaseLoggedIn ? "Clear Session" : "Import Cookie"
                                onClicked: {
                                    if (settingsVm.isNeteaseLoggedIn) {
                                        settingsVm.logoutNetease()
                                    } else {
                                        loginDialog.open()
                                    }
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                            }
                        }

                        Label {
                            visible: settingsVm.hasError
                            text: settingsVm.error.message
                            color: Material.color(Material.Red)
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }
                }

                GroupBox {
                    width: parent.width - 32
                    title: "Storage"

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 12

                        Button {
                            objectName: "clearHistoryButton"
                            text: "Clear Play History"
                            onClicked: settingsVm.clearPlayHistory()
                        }
                    }
                }

                Button {
                    objectName: "aboutButton"
                    text: "About QeriPlayer"
                    onClicked: aboutDialog.open()
                }
            }
        }
    }

    Platform.FolderDialog {
        id: folderDialog
        folder: settingsVm.downloadPathUrl
        onAccepted: settingsVm.setDownloadPath(folder.toLocalFile())
    }

    LoginDialog {
        id: loginDialog
    }

    AboutDialog {
        id: aboutDialog
    }
}
