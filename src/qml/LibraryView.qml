import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Page {
    id: root

    property bool neteasePlaylistsLoaded: false
    property bool neteaseAlbumsLoaded: false

    function ensureRemoteLoaded(forceReload) {
        if (tabBar.currentIndex === 1 && (forceReload || !neteasePlaylistsLoaded)) {
            neteasePlaylistsLoaded = true
            playlistVm.loadNeteasePlaylists()
        } else if (tabBar.currentIndex === 2 && (forceReload || !neteaseAlbumsLoaded)) {
            neteaseAlbumsLoaded = true
            playlistVm.loadNeteaseAlbums()
        }
    }

    function retryCurrentTab() {
        ensureRemoteLoaded(true)
    }

    function localSubtitle(summary) {
        var count = summary.trackCount || 0
        return count + (count === 1 ? " song" : " songs")
    }

    function albumSubtitle(summary) {
        var count = summary.size || 0
        return count + (count === 1 ? " track" : " tracks")
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            color: Material.dialogColor

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16

                Label {
                    text: "Library"
                    font.pixelSize: 24
                    font.weight: Font.DemiBold
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: "New Playlist"
                    visible: tabBar.currentIndex === 0
                    onClicked: createDialog.open()
                }
            }
        }

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            onCurrentIndexChanged: root.ensureRemoteLoaded(false)

            TabButton { text: "Local" }
            TabButton { text: "NetEase Playlists" }
            TabButton { text: "NetEase Albums" }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            Item {
                Label {
                    anchors.centerIn: parent
                    visible: playlistVm.localPlaylists.length === 0
                    text: "No local playlists yet"
                    font.pixelSize: 16
                    color: Material.hintTextColor
                }

                ListView {
                    anchors.fill: parent
                    clip: true
                    model: playlistVm.localPlaylists
                    visible: playlistVm.localPlaylists.length > 0
                    delegate: PlaylistCard {
                        title: modelData.name || ""
                        subtitle: root.localSubtitle(modelData)
                        coverUrl: modelData.coverUrl ? modelData.coverUrl.toString() : ""
                        onClicked: playlistVm.openLocalPlaylist(index)
                    }
                    ScrollBar.vertical: ScrollBar {}
                }
            }

            Item {
                BusyIndicator {
                    anchors.centerIn: parent
                    visible: playlistVm.isLoading
                    running: visible
                }

                Column {
                    anchors.centerIn: parent
                    spacing: 12
                    visible: !playlistVm.isLoading && playlistVm.hasError

                    Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: playlistVm.error.message || "Failed to load playlists"
                        color: Material.hintTextColor
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Button {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Retry"
                        onClicked: root.retryCurrentTab()
                    }
                }

                Label {
                    anchors.centerIn: parent
                    visible: !playlistVm.isLoading && !playlistVm.hasError && playlistVm.neteasePlaylists.length === 0
                    text: neteasePlaylistsLoaded ? "No NetEase playlists found" : "Open this tab to load playlists"
                    font.pixelSize: 16
                    color: Material.hintTextColor
                }

                ListView {
                    anchors.fill: parent
                    clip: true
                    model: playlistVm.neteasePlaylists
                    visible: !playlistVm.hasError && playlistVm.neteasePlaylists.length > 0
                    delegate: PlaylistCard {
                        title: modelData.name || ""
                        subtitle: root.localSubtitle(modelData)
                        coverUrl: modelData.coverUrl ? modelData.coverUrl.toString() : ""
                        onClicked: playlistVm.openNeteasePlaylist(index)
                    }
                    ScrollBar.vertical: ScrollBar {}
                }
            }

            Item {
                BusyIndicator {
                    anchors.centerIn: parent
                    visible: playlistVm.isLoading
                    running: visible
                }

                Column {
                    anchors.centerIn: parent
                    spacing: 12
                    visible: !playlistVm.isLoading && playlistVm.hasError

                    Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: playlistVm.error.message || "Failed to load albums"
                        color: Material.hintTextColor
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Button {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Retry"
                        onClicked: root.retryCurrentTab()
                    }
                }

                Label {
                    anchors.centerIn: parent
                    visible: !playlistVm.isLoading && !playlistVm.hasError && playlistVm.neteaseAlbums.length === 0
                    text: neteaseAlbumsLoaded ? "No NetEase albums found" : "Open this tab to load albums"
                    font.pixelSize: 16
                    color: Material.hintTextColor
                }

                ListView {
                    anchors.fill: parent
                    clip: true
                    model: playlistVm.neteaseAlbums
                    visible: !playlistVm.hasError && playlistVm.neteaseAlbums.length > 0
                    delegate: PlaylistCard {
                        title: modelData.name || ""
                        subtitle: root.albumSubtitle(modelData)
                        coverUrl: modelData.coverUrl ? modelData.coverUrl.toString() : ""
                        onClicked: playlistVm.openNeteaseAlbum(index)
                    }
                    ScrollBar.vertical: ScrollBar {}
                }
            }
        }
    }

    Dialog {
        id: createDialog
        title: "Create Playlist"
        modal: true
        anchors.centerIn: parent
        width: 360
        standardButtons: Dialog.NoButton

        onOpened: playlistNameField.forceActiveFocus()

        ColumnLayout {
            width: parent.width
            spacing: 16

            TextField {
                id: playlistNameField
                Layout.fillWidth: true
                placeholderText: "Playlist name"
                onAccepted: createButton.clicked()
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: "Cancel"
                    onClicked: {
                        playlistNameField.clear()
                        createDialog.close()
                    }
                }

                Button {
                    id: createButton
                    text: "Create"
                    enabled: playlistNameField.text.trim().length > 0
                    onClicked: {
                        playlistVm.createLocalPlaylist(playlistNameField.text.trim())
                        playlistNameField.clear()
                        createDialog.close()
                    }
                }
            }
        }
    }
}
