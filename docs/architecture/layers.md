# Layered Architecture

## 1. Overview

QeriPlayer Qt adopts a classic layered architecture design, dividing the application into presentation layer, business layer, data layer, and infrastructure layer. Each layer has clear responsibilities and well-defined dependencies.

## 2. Architecture Layers

### 2.1 Presentation Layer

The presentation layer is responsible for UI display and user interaction.

**Responsibilities**:
- UI component creation and layout
- User input handling
- Data display
- Animations and visual effects

**Key Components**:
- `MainWindow`: Main window
- `NavigationWidget`: Navigation widget
- `PlayerControlBar`: Player control bar
- `SettingsDialog`: Settings dialog
- Various view pages

**Design Principles**:
- Build interfaces using Qt Widgets or QML
- Bind data through ViewModel
- No business logic included
- Respond to user actions and update UI

### 2.2 Business Layer

The business layer handles the core business logic of the application.

**Responsibilities**:
- Business rule implementation
- Data transformation and processing
- State management
- Module coordination

**Key Components**:
- `MainViewModel`: Main window ViewModel
- `PlayerViewModel`: Player ViewModel
- `SearchViewModel`: Search ViewModel
- `PlaybackController`: Playback orchestration (queue, URL resolution, persistence)

**Design Principles**:
- Follow MVVM pattern
- Use signals and slots for communication
- No direct access to UI components
- ViewModels access API clients and repositories directly
- No dedicated service layer — following the Android QeriPlayer pattern where ViewModels coordinate repos and API clients
- `PlaybackController` is the exception: it encapsulates complex playback orchestration (backend, queue, URL resolution, state persistence) and acts as a de facto service

### 2.3 Data Layer

The data layer is responsible for data retrieval, storage, and management.

**Responsibilities**:
- Data retrieval (API calls)
- Data persistence (database, files)
- Data caching
- Data synchronization

**Key Components**:
- `ISongRepository` / `SongRepository`: Song CRUD on songs_cache
- `IPlaylistRepository` / `PlaylistRepository`: Playlist CRUD + song membership
- `IPlayerStateRepository` / `PlayerStateRepository`: Player state persistence
- `ISettingsRepository` / `SettingsRepository`: Key-value settings
- `IPlayHistoryRepository` / `PlayHistoryRepository`: Play history
- `NeteaseClient`: NetEase Cloud Music client
- `IMusicPlatformPlugin`: Abstract platform interface

**Design Principles**:
- Use Repository pattern
- Unified data access interface
- Support multiple data sources
- Implement data caching strategies

### 2.4 Infrastructure Layer

The infrastructure layer provides low-level technical support.

**Responsibilities**:
- Network communication
- Database storage
- File system operations
- Data encryption
- Logging

**Key Components**:
- `NetworkManager`: Network manager (HttpClient, WebSocketClient, NetworkMonitor)
- `DatabaseManager`: Database manager (sqlite3 C API)
- `AppPaths` / `FileUtils` / `FileWatcher`: Filesystem utilities
- `Encryptor` / `Decryptor` / `SecureStorage` / `CryptoUtils`: Cryptography
- `Logger`: spdlog-based logging with named loggers

**Design Principles**:
- Provide common foundational services
- Platform-agnostic abstract interfaces
- Cross-platform implementation support
- High performance and reliability

## 3. Data Flow

### 3.1 User Action Flow

```
User Action
    ↓
View (UI Component)
    ↓
ViewModel (Business Logic)
    ↓
Repository / API Client (Data Access)
    ↓
Response Data
    ↓
ViewModel (Data Processing)
    ↓
View (UI Update)
```

Note: There is no dedicated service layer. ViewModels access repositories and API clients directly, following the Android QeriPlayer pattern. `PlaybackController` is the one exception — it encapsulates complex playback orchestration and lives in the `player/` module.

### 3.2 Playback Example

```
User clicks Play button
    ↓
PlayerControlBar (UI)
    ↓
PlayerViewModel::play()
    ↓
PlaybackController::play(song)
    ↓
  resolves URL via IMusicPlatformPlugin
    ↓
IPlayerBackend::play(url)
    ↓
Audio playback
    ↓
PlaybackController emits playbackStateChanged signal
    ↓
PlayerViewModel::updateState()
    ↓
PlayerControlBar::updateUI()
```

## 4. Dependencies

### 4.1 Dependency Rules

```
Presentation Layer → Business Layer → Data Layer → Infrastructure Layer
```

- **Presentation Layer** only depends on **Business Layer**
- **Business Layer** only depends on **Data Layer**
- **Data Layer** only depends on **Infrastructure Layer**
- **Infrastructure Layer** does not depend on other layers

### 4.2 Dependency Injection

Using the service locator pattern for dependency injection:

```cpp
// Register service
app.services()->registerService<NetworkManager>(
    std::make_unique<NetworkManager>());

// Get service
auto *network = app.services()->service<NetworkManager>();
```

## 5. Inter-Layer Communication

### 5.1 Downward Calls

The presentation layer calls the business layer through ViewModel, and the ViewModel directly accesses the data layer (repositories and API clients).

```cpp
// Presentation layer calls business layer
void MainWindow::onPlayButtonClicked() {
    m_viewModel->play();
}

// ViewModel calls data layer directly
void PlayerViewModel::play() {
    m_playbackController->play(m_currentSong);
}

// ViewModel calls API client directly
QCoro::Task<void> SearchViewModel::search(const QString &query) {
    auto result = co_await m_neteaseClient->searchSongs(query);
    // process results...
}
```

### 5.2 Upward Notifications

The data layer notifies the business layer through signals, and the business layer notifies the presentation layer through signals.

```cpp
// API client notifies ViewModel directly
connect(m_neteaseClient, &NeteaseClient::requestFinished,
        this, &SearchViewModel::onResultsReady);

// PlaybackController notifies ViewModel
connect(m_playbackController, &PlaybackController::playbackStateChanged,
        this, &PlayerViewModel::onStateChanged);
```

## 6. Module Interaction Examples

### 6.1 Search Songs

```
1. User enters keywords in search box
2. SearchView calls SearchViewModel::search()
3. SearchViewModel calls API clients directly
   - NeteaseClient::searchSongs()
   - BilibiliClient::searchVideos() (future)
   - YouTubeMusicClient::searchSongs() (future)
4. SearchViewModel merges results
5. SearchView displays results
```

Note: Multi-platform search aggregation lives in the ViewModel, not in a dedicated SearchService. This follows the Android QeriPlayer pattern (see HomeViewModel, NowPlayingViewModel).

### 6.2 Play Song

```
1. User clicks a song
2. SongListView calls PlayerViewModel::playSong()
3. PlayerViewModel calls PlaybackController::play()
4. PlaybackController resolves playback URL
   - If local song, play directly
   - If online song, call IMusicPlatformPlugin::getSongUrl()
5. PlaybackController calls IPlayerBackend::play()
6. Audio backend starts playback
7. PlaybackController emits playbackStateChanged signal
8. PlayerViewModel updates UI
9. PlayerControlBar displays playback state
```

## 7. Design Pattern Applications

### 7.1 MVVM Pattern

```cpp
// Model
struct Song {
    QString id;
    QString name;
    QString artist;
};

// ViewModel
class PlayerViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY currentSongChanged)
public:
    QString currentTitle() const { return m_currentSong.name; }
signals:
    void currentSongChanged();
private:
    Song m_currentSong;
};

// View
class PlayerView : public QWidget {
    Q_OBJECT
public:
    void setViewModel(PlayerViewModel *viewModel) {
        m_viewModel = viewModel;
        connect(m_viewModel, &PlayerViewModel::currentSongChanged,
                this, &PlayerView::updateUI);
    }
};
```

### 7.2 Repository Pattern

```cpp
class MusicRepository : public QObject {
    Q_OBJECT
public:
    QCoro::Task<Song> getSong(const QString &id);
    QCoro::Task<Playlist> getPlaylist(const QString &id);
    
private:
    NeteaseClient *m_neteaseClient;
    BilibiliClient *m_bilibiliClient;
    CacheManager *m_cacheManager;
};
```

### 7.3 Service Locator Pattern

```cpp
class ServiceLocator {
public:
    static ServiceLocator* instance();
    
    template<typename T>
    void registerService(std::unique_ptr<T> service);
    
    template<typename T>
    T* service() const;
};
```

## 8. Summary

The layered architecture design gives QeriPlayer Qt a clear structure and good maintainability:
- **Separation of Responsibilities**: Each layer focuses on its own responsibilities
- **Clear Dependencies**: Dependencies are well-defined, facilitating testing and maintenance
- **Extensibility**: Easy to add new features and modules
- **Testability**: Each layer can be tested independently
