## 1. Build System

- [x] 1.1 Enable QCoro::Qml in CMake — set `QCORO_WITH_QML ON`, add `find_package(QCoro6 COMPONENTS Qml)`, link `QCoro::Qml` to `qeriplayer_viewmodel` target
- [x] 1.2 Verify build compiles with QCoro::Qml enabled — run `just build` and confirm no errors

## 2. SearchViewModel

- [x] 2.1 Convert `SearchViewModel::search()` from `void` to `QCoro::QmlTask` return type — update header and source
- [x] 2.2 Convert `SearchViewModel::loadMore()` from `void` to `QCoro::QmlTask` return type — update header and source
- [x] 2.3 Update `TestSearchViewModel` — adjust test calls for QmlTask return (wrap with `QCoro::waitFor()` or test via signals)

## 3. PlaylistViewModel

- [x] 3.1 Convert `PlaylistViewModel::loadLocalPlaylists()` from `void` to `QCoro::QmlTask` return type
- [x] 3.2 Convert `PlaylistViewModel::loadNeteasePlaylists()` from `void` to `QCoro::QmlTask` return type
- [x] 3.3 Convert `PlaylistViewModel::loadNeteaseAlbums()` from `void` to `QCoro::QmlTask` return type
- [x] 3.4 Convert `PlaylistViewModel::createLocalPlaylist()` from `void` to `QCoro::QmlTask` return type
- [x] 3.5 Convert `PlaylistViewModel::deleteLocalPlaylist()` from `void` to `QCoro::QmlTask` return type
- [x] 3.6 Convert `PlaylistViewModel::renameLocalPlaylist()` from `void` to `QCoro::QmlTask` return type
- [x] 3.7 Update `TestPlaylistViewModel` — adjust test calls for QmlTask returns

## 4. LocalPlaylistDetailViewModel

- [x] 4.1 Convert `LocalPlaylistDetailViewModel::loadPlaylist()` from `void` to `QCoro::QmlTask` return type
- [x] 4.2 Convert `LocalPlaylistDetailViewModel::addSong()` from `void` to `QCoro::QmlTask` return type
- [x] 4.3 Convert `LocalPlaylistDetailViewModel::removeSong()` from `void` to `QCoro::QmlTask` return type
- [x] 4.4 Convert `LocalPlaylistDetailViewModel::reorderSongs()` from `void` to `QCoro::QmlTask` return type
- [x] 4.5 Convert `LocalPlaylistDetailViewModel::rename()` from `void` to `QCoro::QmlTask` return type
- [x] 4.6 Convert `LocalPlaylistDetailViewModel::deletePlaylist()` from `void` to `QCoro::QmlTask` return type
- [x] 4.7 Update `TestLocalPlaylistDetailViewModel` — adjust test calls for QmlTask returns

## 5. NeteasePlaylistDetailViewModel

- [x] 5.1 Convert `NeteasePlaylistDetailViewModel::loadPlaylist()` from `void` to `QCoro::QmlTask` return type
- [x] 5.2 Convert `NeteasePlaylistDetailViewModel::loadAlbum()` from `void` to `QCoro::QmlTask` return type
- [x] 5.3 Convert `NeteasePlaylistDetailViewModel::retry()` from `void` to `QCoro::QmlTask` return type
- [x] 5.4 Convert `NeteasePlaylistDetailViewModel::saveToLocal()` from `void` to `QCoro::QmlTask` return type
- [x] 5.5 Update `TestNeteasePlaylistDetailViewModel` — adjust test calls for QmlTask returns

## 6. MainViewModel

- [x] 6.1 Convert `MainViewModel::initialize()` from `void` to `QCoro::QmlTask` return type
- [x] 6.2 Update `TestMainViewModel` — adjust test calls for QmlTask return

## 7. Verification

- [x] 7.1 Run full test suite — `just test` — all tests pass
- [x] 7.2 Run format check — `just check` — no formatting issues
- [x] 7.3 Verify QmlTask works from QML — write a minimal QML snippet that calls `.then()` on a VM method and logs the result (manual or integration test)
