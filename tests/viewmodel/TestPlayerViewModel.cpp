/// @file TestPlayerViewModel.cpp
/// @brief Unit tests for PlayerViewModel with mocked dependencies

#include "domain/Enums.h"
#include "domain/Song.h"
#include "player/PlayQueue.h"
#include "player/PlaybackController.h"
#include "repo/IPlayHistoryRepository.h"
#include "viewmodel/PlayerViewModel.h"

#include <QCoroTask>
#include <QSignalSpy>
#include <QTest>

#include <memory>

using namespace QeriPlayerQt;

// --- Mock IPlayerBackend ---

class MockPlayerBackend : public IPlayerBackend {
    Q_OBJECT

public:
    explicit MockPlayerBackend(QObject *parent = nullptr)
        : IPlayerBackend(parent)
    {
    }

    QCoro::Task<void> load(const QUrl &url) override
    {
        m_url = url;
        co_return;
    }
    void play() override
    {
        m_playing = true;
        m_paused = false;
        Q_EMIT stateChanged(PlaybackState::Playing);
    }
    void pause() override
    {
        m_paused = true;
        Q_EMIT stateChanged(PlaybackState::Paused);
    }
    void stop() override
    {
        m_playing = false;
        m_paused = false;
        Q_EMIT stateChanged(PlaybackState::Stopped);
    }
    void seek(qint64 positionMs) override
    {
        m_position = positionMs;
        Q_EMIT positionChanged(positionMs);
    }
    void setVolume(double normalized) override
    {
        m_volume = normalized;
    }
    void setMuted(bool muted) override
    {
        m_muted = muted;
    }

    PlaybackState state() const override
    {
        if (m_playing && !m_paused)
            return PlaybackState::Playing;
        if (m_paused)
            return PlaybackState::Paused;
        return PlaybackState::Stopped;
    }
    qint64 positionMs() const override
    {
        return m_position;
    }
    qint64 durationMs() const override
    {
        return 180000;
    }
    bool isSeekable() const override
    {
        return true;
    }
    double volume() const override
    {
        return m_volume;
    }
    bool isMuted() const override
    {
        return m_muted;
    }
    QString backendName() const override
    {
        return "Mock";
    }

    bool m_playing = false;
    bool m_paused = false;
    QUrl m_url;
    qint64 m_position = 0;
    double m_volume = 1.0;
    bool m_muted = false;
};

// --- Mock IMusicPlatformPlugin ---

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
        SongUrlResult result;
        result.url = QStringLiteral("https://example.com/song.mp3");
        result.status = SongUrlResult::Status::Success;
        co_return ApiResult<SongUrlResult>(result);
    }
    QCoro::Task<ApiResult<Lyrics>> getLyrics(const QString &) override
    {
        co_return ApiResult<Lyrics>(Lyrics {});
    }
    bool isAuthenticated() const override
    {
        return true;
    }
    QString platformName() const override
    {
        return "Mock";
    }
};

// --- Mock IPlayerStateRepository ---

class MockPlayerStateRepo : public IPlayerStateRepository {
public:
    void save(const PersistedPlayerState &) override
    {
        m_saved = true;
    }
    std::optional<PersistedPlayerState> load() override
    {
        return std::nullopt;
    }
    void clear() override { }
    bool m_saved = false;
};

// --- Mock ISettingsRepository ---

class MockSettingsRepo : public ISettingsRepository {
public:
    std::optional<QString> get(const QString &key) override
    {
        auto it = m_settings.find(key);
        if (it != m_settings.end())
            return it.value().toString();
        return std::nullopt;
    }
    void set(const QString &key, const QString &value) override
    {
        m_settings[key] = value;
    }
    void remove(const QString &key) override
    {
        m_settings.remove(key);
    }
    QVariantMap getAll() override
    {
        return m_settings;
    }
    bool getBool(const QString &key, bool defaultValue) override
    {
        auto it = m_settings.find(key);
        if (it != m_settings.end())
            return it.value() == "true";
        return defaultValue;
    }
    int getInt(const QString &key, int defaultValue) override
    {
        auto it = m_settings.find(key);
        if (it != m_settings.end())
            return it.value().toInt();
        return defaultValue;
    }

    QVariantMap m_settings;
};

// --- Mock IPlayHistoryRepository ---

class MockPlayHistoryRepo : public IPlayHistoryRepository {
public:
    void record(const QString &songId) override
    {
        m_recordedIds.append(songId);
    }
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

    QStringList m_recordedIds;
};

// --- Test class ---

class TestPlayerViewModel : public QObject {
    Q_OBJECT

private:
    Song makeSong(const QString &id, const QString &name)
    {
        Song song;
        song.id = id;
        song.name = name;
        song.artist = "Artist";
        song.platform = MusicPlatform::NetEase;
        return song;
    }

    // Persistent mocks — must outlive the controller
    MockPlugin m_plugin;
    MockPlayerStateRepo m_stateRepo;
    MockSettingsRepo m_settingsRepo;
    MockPlayHistoryRepo m_historyRepo;

    std::unique_ptr<PlaybackController> createController()
    {
        auto backend = std::make_unique<MockPlayerBackend>();
        return std::make_unique<PlaybackController>(std::move(backend), &m_plugin, &m_stateRepo, &m_settingsRepo);
    }

private Q_SLOTS:
    void initialState();
    void play_delegatesToController();
    void pause_delegatesToController();
    void resume_delegatesToController();
    void stop_delegatesToController();
    void volume_setAndGet();
    void toggleMute();
    void cycleRepeatMode();
    void toggleShuffle();
    void queue_addAndRemove();
    void queue_clearQueue();
    void queue_moveInQueue();
    void playHistory_recordedOnSongChange();
    void errorFromController();
};

void TestPlayerViewModel::initialState()
{
    auto controller = createController();
    PlayerViewModel vm(controller.get(), &m_historyRepo);

    QCOMPARE(vm.playbackState(), PlaybackState::Stopped);
    QVERIFY(!vm.isPlaying());
    QVERIFY(!vm.isPaused());
    QVERIFY(!vm.isLoading());
    QCOMPARE(vm.volume(), 1.0);
    QVERIFY(!vm.isMuted());
    QCOMPARE(vm.repeatMode(), RepeatMode::Off);
    QVERIFY(!vm.isShuffleEnabled());
    QVERIFY(!vm.hasError());
}

void TestPlayerViewModel::play_delegatesToController()
{
    auto controller = createController();
    PlayerViewModel vm(controller.get(), &m_historyRepo);
    Song song = makeSong("1", "Test Song");

    QCoro::waitFor(vm.play(song));

    QCOMPARE(controller->playbackState(), PlaybackState::Playing);
    QVERIFY(vm.isPlaying());
}

void TestPlayerViewModel::pause_delegatesToController()
{
    auto controller = createController();
    PlayerViewModel vm(controller.get(), &m_historyRepo);
    Song song = makeSong("1", "Test Song");

    QCoro::waitFor(vm.play(song));
    vm.pause();

    QVERIFY(vm.isPaused());
}

void TestPlayerViewModel::resume_delegatesToController()
{
    auto controller = createController();
    PlayerViewModel vm(controller.get(), &m_historyRepo);
    Song song = makeSong("1", "Test Song");

    QCoro::waitFor(vm.play(song));
    vm.pause();
    vm.resume();

    QVERIFY(vm.isPlaying());
}

void TestPlayerViewModel::stop_delegatesToController()
{
    auto controller = createController();
    PlayerViewModel vm(controller.get(), &m_historyRepo);
    Song song = makeSong("1", "Test Song");

    QCoro::waitFor(vm.play(song));
    vm.stop();

    QCOMPARE(vm.playbackState(), PlaybackState::Stopped);
}

void TestPlayerViewModel::volume_setAndGet()
{
    auto controller = createController();
    PlayerViewModel vm(controller.get(), &m_historyRepo);
    QSignalSpy spy(&vm, &PlayerViewModel::volumeChanged);

    vm.setVolume(0.5);
    QCOMPARE(vm.volume(), 0.5);
    QCOMPARE(spy.count(), 1);
}

void TestPlayerViewModel::toggleMute()
{
    auto controller = createController();
    PlayerViewModel vm(controller.get(), &m_historyRepo);
    QSignalSpy spy(&vm, &PlayerViewModel::mutedChanged);

    QVERIFY(!vm.isMuted());
    vm.toggleMute();
    QVERIFY(vm.isMuted());
    QCOMPARE(spy.count(), 1);

    vm.toggleMute();
    QVERIFY(!vm.isMuted());
    QCOMPARE(spy.count(), 2);
}

void TestPlayerViewModel::cycleRepeatMode()
{
    auto controller = createController();
    PlayerViewModel vm(controller.get(), &m_historyRepo);
    QSignalSpy spy(&vm, &PlayerViewModel::repeatModeChanged);

    QCOMPARE(vm.repeatMode(), RepeatMode::Off);

    vm.cycleRepeatMode();
    QCOMPARE(vm.repeatMode(), RepeatMode::One);
    QCOMPARE(spy.count(), 1);

    vm.cycleRepeatMode();
    QCOMPARE(vm.repeatMode(), RepeatMode::All);
    QCOMPARE(spy.count(), 2);

    vm.cycleRepeatMode();
    QCOMPARE(vm.repeatMode(), RepeatMode::Off);
    QCOMPARE(spy.count(), 3);
}

void TestPlayerViewModel::toggleShuffle()
{
    auto controller = createController();
    PlayerViewModel vm(controller.get(), &m_historyRepo);
    QSignalSpy spy(&vm, &PlayerViewModel::shuffleChanged);

    QVERIFY(!vm.isShuffleEnabled());
    vm.toggleShuffle();
    QVERIFY(vm.isShuffleEnabled());
    QCOMPARE(spy.count(), 1);

    vm.toggleShuffle();
    QVERIFY(!vm.isShuffleEnabled());
    QCOMPARE(spy.count(), 2);
}

void TestPlayerViewModel::queue_addAndRemove()
{
    auto controller = createController();
    PlayerViewModel vm(controller.get(), &m_historyRepo);
    Song song1 = makeSong("1", "Song A");
    Song song2 = makeSong("2", "Song B");

    vm.addToQueue(song1);
    vm.addToQueue(song2);
    QCOMPARE(vm.queue()->count(), 2);

    vm.removeFromQueue(0);
    QCOMPARE(vm.queue()->count(), 1);
    QCOMPARE(vm.queue()->songAt(0).name, QStringLiteral("Song B"));
}

void TestPlayerViewModel::queue_clearQueue()
{
    auto controller = createController();
    PlayerViewModel vm(controller.get(), &m_historyRepo);
    vm.addToQueue(makeSong("1", "A"));
    vm.addToQueue(makeSong("2", "B"));

    vm.clearQueue();
    QCOMPARE(vm.queue()->count(), 0);
}

void TestPlayerViewModel::queue_moveInQueue()
{
    auto controller = createController();
    PlayerViewModel vm(controller.get(), &m_historyRepo);
    vm.addToQueue(makeSong("1", "A"));
    vm.addToQueue(makeSong("2", "B"));
    vm.addToQueue(makeSong("3", "C"));

    vm.moveInQueue(2, 0); // Move C to front

    QCOMPARE(vm.queue()->songAt(0).name, QStringLiteral("C"));
    QCOMPARE(vm.queue()->songAt(1).name, QStringLiteral("A"));
    QCOMPARE(vm.queue()->songAt(2).name, QStringLiteral("B"));
}

void TestPlayerViewModel::playHistory_recordedOnSongChange()
{
    auto controller = createController();
    m_historyRepo.m_recordedIds.clear();

    PlayerViewModel vm(controller.get(), &m_historyRepo);
    Song song = makeSong("abc", "Test Song");

    QCoro::waitFor(vm.play(song));

    QCOMPARE(m_historyRepo.m_recordedIds.size(), 1);
    QCOMPARE(m_historyRepo.m_recordedIds.first(), QStringLiteral("abc"));
}

void TestPlayerViewModel::errorFromController()
{
    auto controller = createController();
    PlayerViewModel vm(controller.get(), &m_historyRepo);
    QSignalSpy spy(&vm, &PlayerViewModel::errorChanged);

    // Simulate error from controller
    Q_EMIT controller->errorOccurred("Network timeout");

    QVERIFY(vm.hasError());
    QCOMPARE(vm.error().message(), QStringLiteral("Network timeout"));
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TestPlayerViewModel)
#include "TestPlayerViewModel.moc"
