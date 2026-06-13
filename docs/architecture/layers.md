# Layered Architecture

## 1. Overview

NeriPlayer Qt adopts a classic layered architecture design, dividing the application into presentation layer, business layer, data layer, and infrastructure layer. Each layer has clear responsibilities and well-defined dependencies.

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
- `PlayerService`: Playback service
- `SearchService`: Search service
- `SyncService`: Sync service

**Design Principles**:
- Follow MVVM pattern
- Use signals and slots for communication
- No direct access to UI components
- Access data through Repository

### 2.3 Data Layer

The data layer is responsible for data retrieval, storage, and management.

**Responsibilities**:
- Data retrieval (API calls)
- Data persistence (database, files)
- Data caching
- Data synchronization

**Key Components**:
- `MusicRepository`: Music repository
- `PlaylistRepository`: Playlist repository
- `SettingsRepository`: Settings repository
- `NeteaseClient`: NetEase client
- `BilibiliClient`: Bilibili client
- `YouTubeMusicClient`: YouTube Music client

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
- `NetworkManager`: Network manager
- `DatabaseManager`: Database manager
- `FileSystemManager`: File system manager
- `CryptoManager`: Crypto manager
- `LogManager`: Log manager

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
Service (Service Layer)
    ↓
Repository (Data Access)
    ↓
API/Storage (Data Source)
    ↓
Response Data
    ↓
ViewModel (Data Processing)
    ↓
View (UI Update)
```

### 3.2 Playback Example

```
User clicks Play button
    ↓
PlayerControlBar (UI)
    ↓
PlayerViewModel::play()
    ↓
PlayerService::play(song)
    ↓
AudioEngine::play(url)
    ↓
Audio playback
    ↓
PlayerService::onPlaybackStarted()
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
ServiceLocator::instance()->registerService<NetworkManager>(
    std::make_unique<NetworkManager>());

// Get service
auto *network = ServiceLocator::instance()->service<NetworkManager>();
```

## 5. Inter-Layer Communication

### 5.1 Downward Calls

The presentation layer calls the business layer through ViewModel, and the business layer calls the data layer through Repository.

```cpp
// Presentation layer calls business layer
void MainWindow::onPlayButtonClicked() {
    m_viewModel->play();
}

// Business layer calls data layer
void PlayerViewModel::play() {
    m_playerService->play(m_currentSong);
}
```

### 5.2 Upward Notifications

The data layer notifies the business layer through signals, and the business layer notifies the presentation layer through signals.

```cpp
// Data layer notifies business layer
connect(m_apiClient, &ApiClient::requestFinished,
        this, &MusicRepository::onRequestFinished);

// Business layer notifies presentation layer
connect(m_playerService, &PlayerService::stateChanged,
        this, &PlayerViewModel::onStateChanged);
```

## 6. Module Interaction Examples

### 6.1 Search Songs

```
1. User enters keywords in search box
2. SearchView calls SearchViewModel::search()
3. SearchViewModel calls SearchService::search()
4. SearchService calls multiple API clients
   - NeteaseClient::searchSongs()
   - BilibiliClient::searchVideos()
   - YouTubeMusicClient::searchSongs()
5. API clients return results
6. SearchService merges results
7. SearchViewModel updates data
8. SearchView displays results
```

### 6.2 Play Song

```
1. User clicks a song
2. SongListView calls PlayerViewModel::playSong()
3. PlayerViewModel calls PlayerService::play()
4. PlayerService resolves playback URL
   - If local song, play directly
   - If online song, call API for URL
5. PlayerService calls AudioEngine::play()
6. AudioEngine starts playback
7. PlayerService updates playback state
8. PlayerViewModel updates UI
9. PlayerControlBar displays playback state
```

## 7. Design Pattern Applications

### 7.1 MVVM Pattern

```cpp
// Model
struct Song {
    QString id;
    QString title;
    QString artist;
};

// ViewModel
class PlayerViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY currentSongChanged)
public:
    QString currentTitle() const { return m_currentSong.title; }
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
    QFuture<Song> getSong(const QString &id);
    QFuture<Playlist> getPlaylist(const QString &id);
    
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

The layered architecture design gives NeriPlayer Qt a clear structure and good maintainability:
- **Separation of Responsibilities**: Each layer focuses on its own responsibilities
- **Clear Dependencies**: Dependencies are well-defined, facilitating testing and maintenance
- **Extensibility**: Easy to add new features and modules
- **Testability**: Each layer can be tested independently
