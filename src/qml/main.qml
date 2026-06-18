import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1000
    height: 700
    visible: true
    title: "QeriPlayer Qt"
    Material.theme: Material.Dark
    Material.accent: Material.Purple

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Sidebar {
                id: sidebar
                Layout.fillHeight: true
                Layout.preferredWidth: 200
            }

            StackView {
                id: contentStack
                Layout.fillWidth: true
                Layout.fillHeight: true
                initialItem: homePage
            }
        }

        PlayerBar {
            id: playerBar
            Layout.fillWidth: true
        }
    }

    Toast {
        id: toast
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: playerBar.top
        anchors.bottomMargin: 8
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        z: 100
    }

    Shortcut {
        sequence: "Space"
        onActivated: {
            if (playerVm.isPlaying)
                playerVm.pause()
            else
                playerVm.resume()
        }
    }

    Shortcut {
        sequence: "Left"
        onActivated: playerVm.seek(Math.max(0, playerVm.positionMs - 5000))
    }

    Shortcut {
        sequence: "Right"
        onActivated: playerVm.seek(Math.min(playerVm.durationMs, playerVm.positionMs + 5000))
    }

    Connections {
        target: playerVm
        function onErrorChanged() {
            if (playerVm.hasError) {
                toast.show(playerVm.error.message)
            }
        }
    }

    Connections {
        target: searchVm
        function onErrorChanged() {
            if (searchVm.hasError) {
                toast.show(searchVm.error.message)
            }
        }
    }

    Connections {
        target: playlistVm
        function onErrorChanged() {
            if (playlistVm.hasError) {
                toast.show(playlistVm.error.message)
            }
        }
    }

    Connections {
        target: mainVm.localPlaylistDetail
        ignoreUnknownSignals: true
        function onErrorChanged() {
            if (mainVm.localPlaylistDetail && mainVm.localPlaylistDetail.hasError) {
                toast.show(mainVm.localPlaylistDetail.error.message)
            }
        }
    }

    Connections {
        target: mainVm.neteasePlaylistDetail
        ignoreUnknownSignals: true
        function onErrorChanged() {
            if (mainVm.neteasePlaylistDetail && mainVm.neteasePlaylistDetail.hasError) {
                toast.show(mainVm.neteasePlaylistDetail.error.message)
            }
        }
    }

    Component {
        id: homePage
        Rectangle {
            color: "transparent"
            Label {
                anchors.centerIn: parent
                text: "Home"
                font.pixelSize: 24
                opacity: 0.5
            }
        }
    }

    Component {
        id: searchPage
        SearchView {}
    }

    Component {
        id: libraryPage
        LibraryView {}
    }

    Component {
        id: localPlaylistPage
        LocalPlaylistDetailView {}
    }

    Component {
        id: neteasePlaylistPage
        NeteasePlaylistDetailView {}
    }

    Component {
        id: settingsPage
        Rectangle {
            color: "transparent"
            Label {
                anchors.centerIn: parent
                text: "Settings (PR 5)"
                font.pixelSize: 24
                opacity: 0.5
            }
        }
    }

    Connections {
        target: mainVm

        function onCurrentViewChanged() {
            var page
            switch (mainVm.currentView) {
            case 0:
                page = homePage
                break
            case 1:
                page = searchPage
                break
            case 2:
                page = libraryPage
                break
            case 3:
                page = localPlaylistPage
                break
            case 4:
                page = neteasePlaylistPage
                break
            case 5:
                page = settingsPage
                break
            default:
                page = homePage
                break
            }
            contentStack.replace(page)
        }
    }
}
