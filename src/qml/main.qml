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

        // Main content area: sidebar + stack
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

        // Player bar
        PlayerBar {
            id: playerBar
            Layout.fillWidth: true
        }
    }

    // Toast notification (overlay above player bar)
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

    // Keyboard shortcuts
    Shortcut {
        sequence: "Space"
        onActivated: {
            if (playerVm.isPlaying) playerVm.pause()
            else playerVm.resume()
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

    // Wire toast to player errors
    Connections {
        target: playerVm
        function onErrorChanged() {
            if (playerVm.hasError) {
                toast.show(playerVm.error.message)
            }
        }
    }

    // Wire toast to search errors
    Connections {
        target: searchVm
        function onErrorChanged() {
            if (searchVm.hasError) {
                toast.show(searchVm.error.message)
            }
        }
    }

    // Placeholder pages
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
        Rectangle {
            color: "transparent"
            Label {
                anchors.centerIn: parent
                text: "Library (PR 4)"
                font.pixelSize: 24
                opacity: 0.5
            }
        }
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

    // Navigation handler
    // NOTE: Case values mirror MainViewModel::View enum (Home=0, Search=1, Library=2, Settings=5).
    // If the enum order changes in C++, update these values to match.
    Connections {
        target: mainVm

        function onCurrentViewChanged() {
            var page;
            switch (mainVm.currentView) {
            case 0: // Home
                page = homePage;
                break;
            case 1: // Search
                page = searchPage;
                break;
            case 2: // Library
                page = libraryPage;
                break;
            case 5: // Settings
                page = settingsPage;
                break;
            default:
                page = homePage;
                break;
            }
            contentStack.replace(page);
        }
    }
}
