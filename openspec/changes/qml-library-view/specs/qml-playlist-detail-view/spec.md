## ADDED Requirements

### Requirement: Local playlist detail page
The system SHALL define `LocalPlaylistDetailView.qml` as a QML Page bound to `mainVm.localPlaylistDetail`.

#### Scenario: Page bound to local detail VM
- **WHEN** the user opens a local playlist
- **THEN** the page SHALL display the playlist name and song list from `mainVm.localPlaylistDetail`

### Requirement: NetEase playlist detail page
The system SHALL define `NeteasePlaylistDetailView.qml` as a QML Page bound to `mainVm.neteasePlaylistDetail`.

#### Scenario: Page bound to NetEase detail VM
- **WHEN** the user opens a NetEase playlist or album
- **THEN** the page SHALL display the header and songs from `mainVm.neteasePlaylistDetail`

### Requirement: Back navigation
Each detail page SHALL provide a Back action that returns to the Library view.

#### Scenario: Back returns to library
- **WHEN** the user taps Back on a detail page
- **THEN** the app SHALL navigate to the Library view and clear the active detail ViewModel

### Requirement: Song list playback
The detail pages SHALL reuse `SongDelegate.qml` for songs and SHALL support single-song and play-all playback.

#### Scenario: Play all from local detail
- **WHEN** the user activates Play All on a local playlist detail page
- **THEN** `mainVm.localPlaylistDetail.playAll()` SHALL be called

#### Scenario: Double-click song
- **WHEN** the user double-clicks a song in a detail page
- **THEN** the corresponding VM SHALL play that song

### Requirement: Local playlist management actions
The local playlist detail page SHALL provide rename, delete, and song removal actions.

#### Scenario: Rename playlist
- **WHEN** the user confirms a rename in the local detail page
- **THEN** `mainVm.localPlaylistDetail.rename(newName)` SHALL be called

#### Scenario: Delete playlist
- **WHEN** the user confirms delete in the local detail page
- **THEN** `mainVm.localPlaylistDetail.deletePlaylist()` SHALL be called

### Requirement: NetEase save-to-local action
The NetEase detail page SHALL provide a Save to Local action.

#### Scenario: Save to local
- **WHEN** the user activates Save to Local on the NetEase detail page
- **THEN** `mainVm.neteasePlaylistDetail.saveToLocal()` SHALL be called
