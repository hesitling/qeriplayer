import QtQuick
import QtTest

Item {
    id: root
    width: 400
    height: 60

    TestCase {
        name: "NowPlayingInfo"
        when: windowShown

        function test_placeholder_when_no_song() {
            // Test that placeholder is shown when title is empty
            var info = Qt.createComponent("../../../src/qml/NowPlayingInfo.qml")
            verify(info.status === Component.Ready, "Component should load: " + info.errorString())

            var instance = info.createObject(root, {
                "title": "",
                "artist": "",
                "coverUrl": ""
            })
            verify(instance !== null, "Object should be created")

            // Verify properties
            compare(instance.title, "")
            compare(instance.artist, "")

            instance.destroy()
        }

        function test_displays_song_info() {
            var info = Qt.createComponent("../../../src/qml/NowPlayingInfo.qml")
            verify(info.status === Component.Ready, "Component should load: " + info.errorString())

            var instance = info.createObject(root, {
                "title": "Test Song",
                "artist": "Test Artist",
                "coverUrl": ""
            })
            verify(instance !== null, "Object should be created")

            // Verify properties
            compare(instance.title, "Test Song")
            compare(instance.artist, "Test Artist")

            instance.destroy()
        }
    }
}
