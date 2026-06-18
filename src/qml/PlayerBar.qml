import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: root

    // --- Properties from playerVm ---
    property bool hasSong: playerVm && playerVm.currentSong && playerVm.currentSong.id !== ""
    property bool isPlaying: playerVm ? playerVm.isPlaying : false
    property bool isLoading: playerVm ? playerVm.isLoading : false
    property bool isShuffleEnabled: playerVm ? playerVm.isShuffleEnabled : false
    property int repeatMode: playerVm ? playerVm.repeatMode : 0
    property int positionMs: playerVm ? playerVm.positionMs : 0
    property int durationMs: playerVm ? playerVm.durationMs : 0
    property double volume: playerVm ? playerVm.volume : 1.0
    property bool isMuted: playerVm ? playerVm.isMuted : false

    // Song info
    property string songTitle: hasSong ? playerVm.currentSong.name : ""
    property string songArtist: hasSong ? playerVm.currentSong.artist : ""
    property url songCoverUrl: hasSong ? playerVm.currentSong.coverUrl : ""

    Layout.fillWidth: true
    Layout.preferredHeight: 80
    color: Material.dialogColor

    // Main layout
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 16

        // Now playing info (left)
        NowPlayingInfo {
            Layout.preferredWidth: 250
            Layout.fillHeight: true
            title: root.songTitle
            artist: root.songArtist
            coverUrl: root.songCoverUrl
        }

        // Transport controls + seek (center)
        TransportControls {
            Layout.fillWidth: true
            Layout.fillHeight: true
            isPlaying: root.isPlaying
            isLoading: root.isLoading
            isShuffleEnabled: root.isShuffleEnabled
            repeatMode: root.repeatMode
            positionMs: root.positionMs
            durationMs: root.durationMs

            onPlayPause: {
                if (root.isPlaying) playerVm.pause()
                else playerVm.resume()
            }
            onNext: playerVm.next()
            onPrev: playerVm.prev()
            onShuffleToggled: playerVm.toggleShuffle()
            onRepeatCycled: playerVm.cycleRepeatMode()
            onSeekRequested: function(pos) { playerVm.seek(pos) }
        }

        // Volume control (right)
        VolumeControl {
            Layout.preferredWidth: 150
            Layout.fillHeight: true
            volume: root.volume
            isMuted: root.isMuted

            onVolumeChangeRequested: function(vol) { playerVm.volume = vol }
            onMuteToggled: playerVm.toggleMute()
        }
    }

    // Disable all controls when no song loaded
    // (individual components handle their own disabled states via hasSong)
}
