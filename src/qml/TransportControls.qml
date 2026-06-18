import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

ColumnLayout {
    id: root

    // --- Playback state properties ---
    property bool isPlaying: false
    property bool isLoading: false
    property bool isShuffleEnabled: false
    property int repeatMode: 0 // 0=Off, 1=One, 2=All

    // --- Seek properties ---
    property int positionMs: 0
    property int durationMs: 0

    // --- Signals ---
    signal playPause()
    signal next()
    signal prev()
    signal shuffleToggled()
    signal repeatCycled()
    signal seekRequested(int positionMs)

    spacing: 0

    // Time formatting helper
    function formatTime(ms) {
        if (ms <= 0) return "0:00"
        var totalSeconds = Math.floor(ms / 1000)
        var hours = Math.floor(totalSeconds / 3600)
        var minutes = Math.floor((totalSeconds % 3600) / 60)
        var seconds = totalSeconds % 60
        if (hours > 0) {
            return hours + ":" + (minutes < 10 ? "0" : "") + minutes + ":" + (seconds < 10 ? "0" : "") + seconds
        }
        return minutes + ":" + (seconds < 10 ? "0" : "") + seconds
    }

    // Transport buttons row
    RowLayout {
        Layout.alignment: Qt.AlignHCenter
        spacing: 4

        // Shuffle
        ToolButton {
            icon.name: "media-playlist-shuffle"
            icon.width: 18
            icon.height: 18
            opacity: root.isShuffleEnabled ? 1.0 : 0.4
            onClicked: root.shuffleToggled()
            ToolTip.text: root.isShuffleEnabled ? "Shuffle on" : "Shuffle off"
            ToolTip.visible: hovered
        }

        // Previous
        ToolButton {
            icon.name: "media-skip-backward"
            icon.width: 24
            icon.height: 24
            onClicked: root.prev()
            ToolTip.text: "Previous"
            ToolTip.visible: hovered
        }

        // Play/Pause (larger)
        ToolButton {
            Layout.preferredWidth: 48
            Layout.preferredHeight: 48
            icon.name: root.isPlaying ? "media-playback-pause" : "media-playback-start"
            icon.width: 32
            icon.height: 32
            onClicked: root.playPause()
            ToolTip.text: root.isPlaying ? "Pause" : "Play"
            ToolTip.visible: hovered
        }

        // Next
        ToolButton {
            icon.name: "media-skip-forward"
            icon.width: 24
            icon.height: 24
            onClicked: root.next()
            ToolTip.text: "Next"
            ToolTip.visible: hovered
        }

        // Repeat
        ToolButton {
            icon.name: root.repeatMode === 1 ? "media-playlist-repeat-song" : "media-playlist-repeat"
            icon.width: 18
            icon.height: 18
            opacity: root.repeatMode === 0 ? 0.4 : 1.0
            onClicked: root.repeatCycled()
            ToolTip.text: root.repeatMode === 0 ? "Repeat off" : (root.repeatMode === 1 ? "Repeat one" : "Repeat all")
            ToolTip.visible: hovered
        }
    }

    // Seek bar row
    RowLayout {
        Layout.fillWidth: true
        spacing: 8

        // Position label
        Label {
            Layout.preferredWidth: 45
            text: formatTime(root.positionMs)
            font.pixelSize: 10
            color: Material.hintTextColor
            horizontalAlignment: Text.AlignRight
        }

        // Seek slider with drag-latch
        Slider {
            id: seekSlider
            Layout.fillWidth: true
            from: 0
            to: root.durationMs > 0 ? root.durationMs : 1
            enabled: root.durationMs > 0

            property bool seeking: false

            value: seeking ? value : root.positionMs

            onMoved: {
                seeking = false
                root.seekRequested(Math.round(value))
            }

            onPressedChanged: {
                if (pressed) {
                    seeking = true
                }
            }
        }

        // Duration label
        Label {
            Layout.preferredWidth: 45
            text: formatTime(root.durationMs)
            font.pixelSize: 10
            color: Material.hintTextColor
        }
    }
}
