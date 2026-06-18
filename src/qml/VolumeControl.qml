import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

RowLayout {
    id: root

    // --- Properties ---
    property double volume: 1.0
    property bool isMuted: false

    // --- Signals ---
    signal volumeChangeRequested(double volume)
    signal muteToggled()

    spacing: 4

    // Volume icon logic
    function volumeIcon() {
        if (root.isMuted) return "volume_off"
        if (root.volume < 0.33) return "volume_mute"
        if (root.volume < 0.66) return "volume_down"
        return "volume_up"
    }

    // Mute toggle button
    ToolButton {
        icon.name: volumeIcon()
        icon.width: 20
        icon.height: 20
        onClicked: root.muteToggled()
        ToolTip.text: root.isMuted ? "Unmute" : "Mute"
        ToolTip.visible: hovered
    }

    // Volume slider
    Slider {
        Layout.preferredWidth: 100
        from: 0.0
        to: 1.0
        value: root.isMuted ? 0.0 : root.volume
        onMoved: root.volumeChangeRequested(value)
    }
}
