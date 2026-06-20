import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Qt.labs.platform as Platform

Page {
    id: root

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
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

        // Scrollable content
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

                // ─── General ──────────────────────────────────────
                GroupBox {
                    width: parent.width - 32
                    title: "General"

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 16

                        // Theme
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

                        // Audio Quality
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Label {
                                text: "Audio Quality"
                                Layout.preferredWidth: 120
                            }

                            ComboBox {
                                Layout.fillWidth: true
                                model: ["Low", "Standard", "High", "Lossless"]
                                currentIndex: settingsVm.audioQuality
                                onActivated: index => {
                                    settingsVm.setAudioQuality(index)
                                }
                            }
                        }

                        // Download Path
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

                // ─── NetEase Account ─────────────────────────────
                GroupBox {
                    width: parent.width - 32
                    title: "NetEase Account"

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 12

                        Label {
                            text: settingsVm.isNeteaseLoggedIn ? "Logged in as " + settingsVm.neteaseUsername : "Not logged in"
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Button {
                                text: settingsVm.isNeteaseLoggedIn ? "Logout" : "Login"
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

                // ─── Storage ─────────────────────────────────────
                GroupBox {
                    width: parent.width - 32
                    title: "Storage"

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 12

                        Button {
                            text: "Clear Play History"
                            onClicked: settingsVm.clearPlayHistory()
                        }
                    }
                }

                // ─── About ───────────────────────────────────────
                Button {
                    text: "About QeriPlayer"
                    onClicked: aboutDialog.open()
                }
            }
        }
    }

    Platform.FolderDialog {
        id: folderDialog
        folder: settingsVm.downloadPath ? "file://" + settingsVm.downloadPath : ""
        onAccepted: {
            var path = folder.toString()
            // Remove "file://" prefix
            if (path.startsWith("file://")) {
                path = path.substring(7)
            }
            settingsVm.setDownloadPath(path)
        }
    }

    LoginDialog {
        id: loginDialog
    }

    AboutDialog {
        id: aboutDialog
    }
}
