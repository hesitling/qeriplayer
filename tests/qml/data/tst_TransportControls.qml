import QtQuick
import QtTest

Item {
    id: root
    width: 400
    height: 100

    TestCase {
        name: "TransportControls"
        when: windowShown

        function test_initial_state() {
            var ctrl = Qt.createComponent("../../../src/qml/TransportControls.qml")
            verify(ctrl.status === Component.Ready, "Component should load: " + ctrl.errorString())

            var instance = ctrl.createObject(root, {
                "isPlaying": false,
                "isLoading": false,
                "isShuffleEnabled": false,
                "repeatMode": 0,
                "positionMs": 0,
                "durationMs": 0
            })
            verify(instance !== null, "Object should be created")

            // Verify initial properties
            compare(instance.isPlaying, false)
            compare(instance.isLoading, false)
            compare(instance.isShuffleEnabled, false)
            compare(instance.repeatMode, 0)
            compare(instance.positionMs, 0)
            compare(instance.durationMs, 0)

            instance.destroy()
        }

        function test_playing_state() {
            var ctrl = Qt.createComponent("../../../src/qml/TransportControls.qml")
            verify(ctrl.status === Component.Ready, "Component should load: " + ctrl.errorString())

            var instance = ctrl.createObject(root, {
                "isPlaying": true,
                "isLoading": false,
                "isShuffleEnabled": true,
                "repeatMode": 2,
                "positionMs": 30000,
                "durationMs": 180000
            })
            verify(instance !== null, "Object should be created")

            // Verify playing state
            compare(instance.isPlaying, true)
            compare(instance.isShuffleEnabled, true)
            compare(instance.repeatMode, 2) // Repeat All
            compare(instance.positionMs, 30000)
            compare(instance.durationMs, 180000)

            instance.destroy()
        }

        function test_formatTime() {
            var ctrl = Qt.createComponent("../../../src/qml/TransportControls.qml")
            verify(ctrl.status === Component.Ready, "Component should load: " + ctrl.errorString())

            var instance = ctrl.createObject(root, {
                "isPlaying": false,
                "positionMs": 0,
                "durationMs": 0
            })
            verify(instance !== null, "Object should be created")

            // Test formatTime function
            compare(instance.formatTime(0), "0:00")
            compare(instance.formatTime(1000), "0:01")
            compare(instance.formatTime(60000), "1:00")
            compare(instance.formatTime(90000), "1:30")
            compare(instance.formatTime(3600000), "1:00:00")
            compare(instance.formatTime(3661000), "1:01:01")

            instance.destroy()
        }

        function test_seek_emitsDraggedPositionAfterPlaybackUpdate() {
            var ctrl = Qt.createComponent("../../../src/qml/TransportControls.qml")
            verify(ctrl.status === Component.Ready, "Component should load: " + ctrl.errorString())

            var instance = ctrl.createObject(root, {
                "positionMs": 30000,
                "durationMs": 180000
            })
            verify(instance !== null, "Object should be created")
            wait(0)

            var slider = findChild(instance, "seekSlider")
            verify(slider !== null, "Seek slider should be available")
            var requestedPositions = []
            function recordSeek(positionMs) {
                requestedPositions.push(positionMs)
            }
            instance.seekRequested.connect(recordSeek)

            mousePress(slider, slider.width / 6, slider.height / 2)
            verify(slider.seeking)

            mouseMove(slider, slider.width / 2, slider.height / 2, 0, Qt.LeftButton)
            verify(slider.pendingPosition > 80000 && slider.pendingPosition < 100000,
                   "Drag should choose the midpoint position")

            instance.positionMs = 45000
            compare(Math.round(slider.value), Math.round(slider.pendingPosition))

            mouseRelease(slider, slider.width / 2, slider.height / 2)
            compare(requestedPositions.length, 1)
            verify(requestedPositions[0] > 80000 && requestedPositions[0] < 100000,
                   "Release must seek to the dragged position, not the playback update")
            verify(!slider.seeking)

            instance.seekRequested.disconnect(recordSeek)
            instance.destroy()
        }
    }
}
