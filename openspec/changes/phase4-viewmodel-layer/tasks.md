## 1. Foundation Types

- [x] 1.1 Create `src/viewmodel/ViewModelError.h` — Q_GADGET with ErrorType enum, Q_PROPERTYs, factory methods
- [x] 1.2 Create `src/viewmodel/ViewModelError.cpp` — implement fromApiError, network, database, validation factories and canRetry logic
- [x] 1.3 Create `src/viewmodel/SongListModel.h` — QAbstractListModel subclass with song roles enum, Q_PROPERTY count
- [x] 1.4 Create `src/viewmodel/SongListModel.cpp` — implement rowCount, data, roleNames, setSongs, appendSongs, clear, songAt, setPlayingIndex
- [x] 1.5 Write `tests/viewmodel/TestViewModelError.cpp` — test ErrorType mapping from ApiError, canRetry classification, factory methods
- [x] 1.6 Write `tests/viewmodel/TestSongListModel.cpp` — test setSongs, appendSongs, clear, roles, count property, playing index
- [x] 1.7 Update CMakeLists.txt — add viewmodel library target with ViewModelError and SongListModel, add test targets

## 2. PlayerViewModel

- [x] 2.1 Create `src/viewmodel/PlayerViewModel.h` — Q_PROPERTYs (currentSong, playbackState, isPlaying, isPaused, isLoading, positionMs, durationMs, volume, isMuted, repeatMode, isShuffleEnabled, queue, hasError, error), Q_INVOKABLEs (play, pause, resume, stop, seek, next, prev, toggleMute, cycleRepeatMode, toggleShuffle, addToQueue, removeFromQueue, moveInQueue, clearQueue)
- [x] 2.2 Create `src/viewmodel/PlayerViewModel.cpp` — connect PlaybackController signals to property updates, implement play/pause/resume/stop/seek/next/prev delegation, implement volume/mute/shuffle/repeat via PlaybackController, implement queue management via PlayQueue, record play history via IPlayHistoryRepository on song change, map PlaybackController errors to ViewModelError
- [x] 2.3 Write `tests/viewmodel/TestPlayerViewModel.cpp` — mock PlaybackController and IPlayHistoryRepository, test property signal propagation, test play/pause/seek delegation, test volume/mute/shuffle/repeat, test queue management, test play history recording, test error mapping

## 3. SettingsViewModel

- [ ] 3.1 Create `src/viewmodel/SettingsViewModel.h` — Q_PROPERTYs (theme, audioQuality, downloadPath, isNeteaseLoggedIn, neteaseUsername, hasError, error), Q_INVOKABLEs (loadSettings, setTheme, setAudioQuality, setDownloadPath, loginNetease, logoutNetease, clearPlayHistory, clearError)
- [ ] 3.2 Create `src/viewmodel/SettingsViewModel.cpp` — implement settings read/write via ISettingsRepository, implement NeteaseClient login/logout/auth status, implement loadSettings hydration, map errors to ViewModelError
- [ ] 3.3 Write `tests/viewmodel/TestSettingsViewModel.cpp` — mock ISettingsRepository, NeteaseClient, IPlayHistoryRepository, test settings persistence round-trip, test login/logout flow, test error mapping

## 4. SearchViewModel

- [ ] 4.1 Create `src/viewmodel/SearchViewModel.h` — Q_PROPERTYs (query, selectedPlatform, results as SongListModel*, isLoading, hasMore, hasError, error, availablePlatforms), Q_INVOKABLEs (search, loadMore, clearResults, clearError), signal requestPlay(Song)
- [ ] 4.2 Create `src/viewmodel/SearchViewModel.cpp` — implement debounce QTimer (300ms), implement platform dispatch to IMusicPlatformPlugin, implement request versioning (quint64 counter), implement pagination with offset tracking, cache results via ISongRepository, map ApiResult errors to ViewModelError
- [ ] 4.3 Write `tests/viewmodel/TestSearchViewModel.cpp` — mock IMusicPlatformPlugin with canned ApiResult, test debounce timing, test platform dispatch, test request versioning (stale results discarded), test pagination, test error mapping

## 5. PlaylistViewModel

- [ ] 5.1 Create `src/viewmodel/PlaylistViewModel.h` — Q_PROPERTYs (localPlaylists, neteasePlaylists, neteaseAlbums, isLoading, hasError, error), Q_INVOKABLEs (loadLocalPlaylists, loadNeteasePlaylists, loadNeteaseAlbums, createLocalPlaylist, deleteLocalPlaylist, renameLocalPlaylist, clearError), signals (localPlaylistSelected, neteasePlaylistSelected, neteaseAlbumSelected)
- [ ] 5.2 Create `src/viewmodel/PlaylistViewModel.cpp` — implement local playlists from IPlaylistRepository::findAll(), implement NetEase playlists from NeteaseClient::getUserPlaylists(), implement NetEase albums from NeteaseClient::getUserStarredAlbums(), implement CRUD delegation, map errors to ViewModelError
- [ ] 5.3 Write `tests/viewmodel/TestPlaylistViewModel.cpp` — mock IPlaylistRepository and NeteaseClient, test playlist loading, test CRUD operations, test selection signals, test error mapping

## 6. LocalPlaylistDetailViewModel

- [ ] 6.1 Create `src/viewmodel/LocalPlaylistDetailViewModel.h` — Q_PROPERTYs (playlistId, playlistName, songs as SongListModel*, isLoading, hasError, error), Q_INVOKABLEs (loadPlaylist, addSong, removeSong, reorderSongs, rename, deletePlaylist, playSong, playAll), signals (requestPlay, requestPlayPlaylist, playlistDeleted)
- [ ] 6.2 Create `src/viewmodel/LocalPlaylistDetailViewModel.cpp` — implement load via IPlaylistRepository::findById(), implement song management via IPlaylistRepository, implement play signal emission with songs from model, map errors to ViewModelError
- [ ] 6.3 Write `tests/viewmodel/TestLocalPlaylistDetailViewModel.cpp` — mock IPlaylistRepository and ISongRepository, test playlist loading, test add/remove/reorder, test rename/delete, test play signals, test error when playlist not found

## 7. NeteasePlaylistDetailViewModel

- [ ] 7.1 Create `src/viewmodel/NeteasePlaylistDetailViewModel.h` — Q_PROPERTYs (headerName, headerCoverUrl, headerTrackCount, songs as SongListModel*, isLoading, hasError, error), Q_INVOKABLEs (loadPlaylist, loadAlbum, retry, saveToLocal, playSong, playAll), signals (requestPlay, requestPlayPlaylist)
- [ ] 7.2 Create `src/viewmodel/NeteasePlaylistDetailViewModel.cpp` — implement playlist fetch via NeteaseClient::getPlaylistDetail(), implement album fetch via NeteaseClient::getAlbumDetail(), implement batch-fetch for large playlists (>1000 tracks in pages of 300), implement song caching via ISongRepository::saveBatch(), implement saveToLocal (create local playlist + duplicate songs), implement retry, map errors to ViewModelError
- [ ] 7.3 Write `tests/viewmodel/TestNeteasePlaylistDetailViewModel.cpp` — mock NeteaseClient, ISongRepository, IPlaylistRepository, test playlist load, test album load, test batch-fetch logic, test saveToLocal, test retry, test error mapping

## 8. MainViewModel

- [ ] 8.1 Create `src/viewmodel/MainViewModel.h` — Q_PROPERTYs (currentView as View enum, playerViewModel, searchViewModel, playlistViewModel, settingsViewModel, localPlaylistDetail, neteasePlaylistDetail), Q_INVOKABLEs (navigateTo, openLocalPlaylist, openNeteasePlaylist, openNeteaseAlbum, initialize), View Q_ENUM
- [ ] 8.2 Create `src/viewmodel/MainViewModel.cpp` — implement child VM ownership, implement signal wiring (search→player, localDetail→player, neteaseDetail→player, playlistVM→navigation), implement detail VM lifecycle (create on navigate, delete on navigate away), implement initialize (loadSettings, restoreState, loadLocalPlaylists)
- [ ] 8.3 Write `tests/viewmodel/TestMainViewModel.cpp` — test navigation enum, test detail VM lifecycle (created/deleted), test signal wiring (search requestPlay reaches player), test initialize sequence

## 9. Build Integration

- [ ] 9.1 Update CMakeLists.txt — add all viewmodel source files to library target, link against api, repo, player, domain targets
- [ ] 9.2 Update CMakeLists.txt test section — add all viewmodel test files, link against viewmodel library and test mocks
- [ ] 9.3 Verify full build — run `just build` and ensure no compilation errors
- [ ] 9.4 Verify all tests pass — run `just test` and ensure all viewmodel tests pass
