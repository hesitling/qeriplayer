import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Dialog {
    id: loginDialog
    title: "Login to NetEase"
    modal: true
    anchors.centerIn: parent
    width: 420
    standardButtons: Dialog.NoButton

    onOpened: {
        tabBar.currentIndex = 0
        settingsVm.clearError()
        phoneField.text = ""
        passField.text = ""
        smsPhoneField.text = ""
        captchaField.text = ""
    }

    onClosed: {
        settingsVm.cancelQrLogin()
        settingsVm.clearError()
    }

    // Watch for successful login
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

        TabBar {
            id: tabBar
            Layout.fillWidth: true

            TabButton {
                text: "Password"
            }
            TabButton {
                text: "SMS Code"
            }
            TabButton {
                text: "QR Code"
                onClicked: {
                    if (visible) {
                        settingsVm.generateQrLogin()
                    }
                }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            // ─── Tab 0: Password ───────────────────────────────
            ColumnLayout {
                spacing: 12

                TextField {
                    id: phoneField
                    Layout.fillWidth: true
                    placeholderText: "Phone number"
                    inputMethodHints: Qt.ImhDigitsOnly
                }

                TextField {
                    id: passField
                    Layout.fillWidth: true
                    placeholderText: "Password"
                    echoMode: TextInput.Password
                    onAccepted: passwordLoginBtn.clicked()
                }

                Label {
                    visible: settingsVm.hasError && tabBar.currentIndex === 0
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
                        text: "Cancel"
                        onClicked: loginDialog.close()
                    }

                    Button {
                        id: passwordLoginBtn
                        text: "Login"
                        enabled: phoneField.text.length > 0 && passField.text.length > 0
                        onClicked: {
                            settingsVm.clearError()
                            settingsVm.loginByPassword(phoneField.text, passField.text)
                        }
                    }
                }
            }

            // ─── Tab 1: SMS Code ───────────────────────────────
            ColumnLayout {
                spacing: 12

                TextField {
                    id: smsPhoneField
                    Layout.fillWidth: true
                    placeholderText: "Phone number"
                    inputMethodHints: Qt.ImhDigitsOnly
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    TextField {
                        id: captchaField
                        Layout.fillWidth: true
                        placeholderText: "Verification code"
                        inputMethodHints: Qt.ImhDigitsOnly
                        onAccepted: smsLoginBtn.clicked()
                    }

                    Button {
                        text: settingsVm.canSendCaptcha ? "Send Code" : "Resend (" + settingsVm.captchaCooldown + "s)"
                        enabled: settingsVm.canSendCaptcha && smsPhoneField.text.length > 0
                        onClicked: {
                            settingsVm.clearError()
                            settingsVm.sendCaptcha(smsPhoneField.text)
                        }
                    }
                }

                Label {
                    visible: settingsVm.hasError && tabBar.currentIndex === 1
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
                        text: "Cancel"
                        onClicked: loginDialog.close()
                    }

                    Button {
                        id: smsLoginBtn
                        text: "Login"
                        enabled: smsPhoneField.text.length > 0 && captchaField.text.length > 0
                        onClicked: {
                            settingsVm.clearError()
                            settingsVm.loginByCaptcha(smsPhoneField.text, captchaField.text)
                        }
                    }
                }
            }

            // ─── Tab 2: QR Code ────────────────────────────────
            ColumnLayout {
                spacing: 16

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    // QR Code image
                    Image {
                        id: qrImage
                        anchors.centerIn: parent
                        width: 200
                        height: 200
                        source: settingsVm.qrImageUrl
                        fillMode: Image.PreserveAspectFit
                        visible: settingsVm.qrLoginStatus !== 0 // Not Expired

                        // Loading indicator
                        BusyIndicator {
                            anchors.centerIn: parent
                            running: qrImage.status === Image.Loading
                        }
                    }

                    // Expired state
                    Column {
                        anchors.centerIn: parent
                        spacing: 12
                        visible: settingsVm.qrLoginStatus === 0 // Expired

                        Label {
                            text: "QR Code Expired"
                            font.pixelSize: 16
                            color: Material.hintTextColor
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Button {
                            text: "Refresh"
                            anchors.horizontalCenter: parent.horizontalCenter
                            onClicked: settingsVm.generateQrLogin()
                        }
                    }
                }

                // Status label
                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: {
                        switch (settingsVm.qrLoginStatus) {
                        case 0: return "Expired"
                        case 1: return "Scan with NetEase app"
                        case 2: return "Scanned — confirm on phone"
                        case 3: return "Confirmed!"
                        default: return ""
                        }
                    }
                    font.pixelSize: 14
                    color: settingsVm.qrLoginStatus === 2 ? Material.accentColor : Material.hintTextColor
                }

                // Polling timer
                Timer {
                    interval: 3000
                    repeat: true
                    running: tabBar.currentIndex === 2 && settingsVm.qrPollingActive
                    onTriggered: settingsVm.pollQrLogin()
                }

                // Error display
                Label {
                    visible: settingsVm.hasError && tabBar.currentIndex === 2
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
                        text: "Cancel"
                        onClicked: loginDialog.close()
                    }
                }
            }
        }
    }
}
