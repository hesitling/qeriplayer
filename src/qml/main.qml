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

        // Placeholder player bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: Material.dialogColor

            Label {
                anchors.centerIn: parent
                text: "Player Bar (PR 2)"
                opacity: 0.5
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
        Rectangle {
            color: "transparent"
            Label {
                anchors.centerIn: parent
                text: "Search (PR 3)"
                font.pixelSize: 24
                opacity: 0.5
            }
        }
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
