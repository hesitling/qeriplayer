import QtQuick
import QtTest

Item {
    id: root
    width: 800
    height: 600

    QtObject {
        id: playlistVm
        property var localPlaylists: []
        property var neteasePlaylists: []
        property var neteaseAlbums: []
        property bool isLoading: false
        property bool hasError: false
        property var error: ({ message: "" })
        property int localOpenCount: 0
        property int remotePlaylistOpenCount: 0
        property int remoteAlbumOpenCount: 0
        property int loadPlaylistCount: 0
        property int loadAlbumCount: 0
        property int createPlaylistCount: 0

        function openLocalPlaylist(index) { localOpenCount += 1 }
        function openNeteasePlaylist(index) { remotePlaylistOpenCount += 1 }
        function openNeteaseAlbum(index) { remoteAlbumOpenCount += 1 }
        function loadNeteasePlaylists() { loadPlaylistCount += 1 }
        function loadNeteaseAlbums() { loadAlbumCount += 1 }
        function createLocalPlaylist(name) { createPlaylistCount += 1 }
    }

    TestCase {
        name: "LibraryView"
        when: windowShown

        function test_component_loads() {
            var component = Qt.createComponent("../../../src/qml/LibraryView.qml")
            verify(component.status === Component.Ready, component.errorString())

            var instance = component.createObject(root)
            verify(instance !== null)
            instance.destroy()
        }
    }
}
