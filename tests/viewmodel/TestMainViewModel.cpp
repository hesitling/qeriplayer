/// @file TestMainViewModel.cpp
/// @brief Unit tests for MainViewModel

#include "viewmodel/MainViewModel.h"
#include "viewmodel/PlayerViewModel.h"
#include "viewmodel/PlaylistViewModel.h"
#include "viewmodel/SearchViewModel.h"
#include "viewmodel/SettingsViewModel.h"

#include <QSignalSpy>
#include <QTest>

using namespace QeriPlayerQt;

// Minimal mocks for testing

class MockPlayerBackend : public IPlayerBackend {
    Q_OBJECT
public:
    explicit MockPlayerBackend(QObject *parent = nullptr)
        : IPlayerBackend(parent)
    {
    }
    QCoro::Task<void> load(const QUrl &) override
    {
        co_return;
    }
    void play() override { }
    void pause() override { }
    void stop() override { }
    void seek(qint64) override { }
    PlaybackState state() const override
    {
        return PlaybackState::Stopped;
    }
    qint64 positionMs() const override
    {
        return 0;
    }
    qint64 durationMs() const override
    {
        return 0;
    }
    bool isSeekable() const override
    {
        return true;
    }
    void setVolume(double) override { }
    double volume() const override
    {
        return 1.0;
    }
    void setMuted(bool) override { }
    bool isMuted() const override
    {
        return false;
    }
    QString backendName() const override
    {
        return "Mock";
    }
};

class MockSettingsRepo : public ISettingsRepository {
public:
    std::optional<QString> get(const QString &) override
    {
        return std::nullopt;
    }
    void set(const QString &, const QString &) override { }
    void remove(const QString &) override { }
    QVariantMap getAll() override
    {
        return {};
    }
    bool getBool(const QString &, bool defaultValue) override
    {
        return defaultValue;
    }
    int getInt(const QString &, int defaultValue) override
    {
        return defaultValue;
    }
};

class MockPlayerStateRepo : public IPlayerStateRepository {
public:
    void save(const PersistedPlayerState &) override { }
    std::optional<PersistedPlayerState> load() override
    {
        return std::nullopt;
    }
    void clear() override { }
};

class MockHistoryRepo : public IPlayHistoryRepository {
public:
    void record(const QString &) override { }
    QVector<Song> recent(int) override
    {
        return {};
    }
    void clear() override { }
    void remove(const QStringList &) override { }
    int playCount(const QString &) override
    {
        return 0;
    }
};

class MockPlaylistRepo : public IPlaylistRepository {
public:
    QVector<PlaylistSummary> findAll() override
    {
        return {};
    }
    std::optional<Playlist> findById(const QString &) override
    {
        return std::nullopt;
    }
    Playlist create(const QString &, MusicPlatform) override
    {
        return {};
    }
    void updateMetadata(const QString &, const QString &, const QString &, const QString &) override { }
    void remove(const QString &) override { }
    bool addSong(const QString &, const QString &, int) override
    {
        return true;
    }
    void removeSong(const QString &, const QString &) override { }
    void reorderSongs(const QString &, const QStringList &) override { }
    int songCount(const QString &) override
    {
        return 0;
    }
};

class MockSongRepo : public ISongRepository {
public:
    std::optional<Song> findById(const QString &) override
    {
        return std::nullopt;
    }
    QVector<Song> findByIds(const QStringList &) override
    {
        return {};
    }
    void save(const Song &) override { }
    void saveBatch(const QVector<Song> &) override { }
    void remove(const QString &) override { }
    bool exists(const QString &) override
    {
        return false;
    }
    QVector<Song> findByPlatform(MusicPlatform) override
    {
        return {};
    }
    QVector<Song> search(const QString &, int) override
    {
        return {};
    }
};

class MockPlugin : public IMusicPlatformPlugin {
public:
    QCoro::Task<ApiResult<SearchResult>> search(const QString &, SearchType, int, int) override
    {
        co_return ApiResult<SearchResult>(SearchResult {});
    }
    QCoro::Task<ApiResult<Song>> getSongDetail(const QString &) override
    {
        co_return ApiResult<Song>(Song {});
    }
    QCoro::Task<ApiResult<SongUrlResult>> getSongUrl(const QString &, AudioQuality) override
    {
        co_return ApiResult<SongUrlResult>(SongUrlResult {});
    }
    QCoro::Task<ApiResult<Lyrics>> getLyrics(const QString &) override
    {
        co_return ApiResult<Lyrics>(Lyrics {});
    }
    bool isAuthenticated() const override
    {
        return false;
    }
    QString platformName() const override
    {
        return "Mock";
    }
};

class TestMainViewModel : public QObject {
    Q_OBJECT

private:
    // Owned by MainViewModel via parent-child
    PlayerViewModel *m_playerVm = nullptr;
    SearchViewModel *m_searchVm = nullptr;
    PlaylistViewModel *m_playlistVm = nullptr;
    SettingsViewModel *m_settingsVm = nullptr;

    // Dependencies (owned by test)
    MockPlayerBackend *m_backend = nullptr;
    MockPlayerStateRepo m_stateRepo;
    MockSettingsRepo m_settingsRepo;
    MockHistoryRepo m_historyRepo;
    MockPlaylistRepo m_playlistRepo;
    MockSongRepo m_songRepo;
    MockPlugin m_plugin;

    MainViewModel *createViewModel()
    {
        m_backend = new MockPlayerBackend();
        auto *controller = new PlaybackController(std::unique_ptr<IPlayerBackend>(m_backend), nullptr, &m_stateRepo,
                                                  &m_settingsRepo);

        m_playerVm = new PlayerViewModel(controller, &m_historyRepo);
        m_searchVm = new SearchViewModel({&m_plugin}, &m_songRepo);
        m_playlistVm = new PlaylistViewModel(&m_playlistRepo, nullptr);
        m_settingsVm = new SettingsViewModel(&m_settingsRepo, nullptr, &m_historyRepo);

        return new MainViewModel(m_playerVm, m_searchVm, m_playlistVm, m_settingsVm, &m_songRepo, &m_playlistRepo,
                                 nullptr);
    }

private Q_SLOTS:
    void initialState();
    void navigation_changesView();
    void searchRequestPlay_wiredToPlayer();
};

void TestMainViewModel::initialState()
{
    auto *vm = createViewModel();

    QCOMPARE(vm->currentView(), MainViewModel::View::Home);
    QVERIFY(vm->playerViewModel() != nullptr);
    QVERIFY(vm->searchViewModel() != nullptr);
    QVERIFY(vm->playlistViewModel() != nullptr);
    QVERIFY(vm->settingsViewModel() != nullptr);
    QVERIFY(vm->localPlaylistDetail() == nullptr);
    QVERIFY(vm->neteasePlaylistDetail() == nullptr);

    delete vm;
}

void TestMainViewModel::navigation_changesView()
{
    auto *vm = createViewModel();

    QSignalSpy spy(vm, &MainViewModel::currentViewChanged);
    vm->navigateTo(MainViewModel::View::Search);

    QCOMPARE(vm->currentView(), MainViewModel::View::Search);
    QCOMPARE(spy.count(), 1);

    delete vm;
}

void TestMainViewModel::searchRequestPlay_wiredToPlayer()
{
    auto *vm = createViewModel();

    QSignalSpy spy(m_playerVm, &PlayerViewModel::currentSongChanged);

    Song song;
    song.id = "test";
    song.name = "Test Song";
    Q_EMIT m_searchVm->requestPlay(song);

    // The signal should be connected to playerVm->play()
    // Since play is async, we just verify the connection exists
    QCOMPARE(spy.count(), 1);

    delete vm;
}

QTEST_MAIN(TestMainViewModel)
#include "TestMainViewModel.moc"
