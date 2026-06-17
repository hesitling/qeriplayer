/// @file TestPlaybackController.cpp
/// @brief Unit tests for PlaybackController with mock backend

#include "player/PlaybackController.h"
#include "player/IPlayerBackend.h"

#include <QCoroTask>
#include <QSignalSpy>
#include <QTest>

using namespace NeriPlayerQt;

namespace {

/**
 * @brief Mock IPlayerBackend for testing PlaybackController
 */
class MockBackend : public IPlayerBackend {
    Q_OBJECT

public:
    explicit MockBackend(QObject *parent = nullptr)
        : IPlayerBackend(parent)
    {
    }

    QCoro::Task<void> load(const QUrl &url) override
    {
        m_lastLoadedUrl = url;
        m_loadCount++;
        co_return;
    }

    void play() override
    {
        m_playCount++;
        Q_EMIT stateChanged(PlaybackState::Playing);
    }

    void pause() override
    {
        m_pauseCount++;
        Q_EMIT stateChanged(PlaybackState::Paused);
    }

    void stop() override
    {
        m_stopCount++;
        Q_EMIT stateChanged(PlaybackState::Stopped);
    }

    void seek(qint64 positionMs) override
    {
        m_lastSeekPosition = positionMs;
        Q_EMIT positionChanged(positionMs);
    }

    PlaybackState state() const override { return m_currentState; }
    qint64 positionMs() const override { return m_position; }
    qint64 durationMs() const override { return m_duration; }
    bool isSeekable() const override { return true; }

    void setVolume(double normalized) override { m_volume = normalized; }
    double volume() const override { return m_volume; }
    void setMuted(bool muted) override { m_muted = muted; }
    bool isMuted() const override { return m_muted; }

    QString backendName() const override { return QStringLiteral("Mock"); }

    // Test helpers
    QUrl m_lastLoadedUrl;
    int m_loadCount = 0;
    int m_playCount = 0;
    int m_pauseCount = 0;
    int m_stopCount = 0;
    qint64 m_lastSeekPosition = 0;
    PlaybackState m_currentState = PlaybackState::Stopped;
    qint64 m_position = 0;
    qint64 m_duration = 300000;
    double m_volume = 1.0;
    bool m_muted = false;
};

/**
 * @brief Mock ISettingsRepository for testing
 */
class MockSettingsRepository : public ISettingsRepository {
public:
    std::optional<QString> get(const QString &key) override
    {
        auto it = m_settings.find(key);
        if (it != m_settings.end()) {
            return it.value().toString();
        }
        return std::nullopt;
    }

    void set(const QString &key, const QString &value) override { m_settings[key] = value; }

    void remove(const QString &key) override { m_settings.remove(key); }

    QVariantMap getAll() override { return m_settings; }

    bool getBool(const QString &key, bool defaultValue) override
    {
        auto val = get(key);
        if (!val.has_value()) {
            return defaultValue;
        }
        return val.value() == QStringLiteral("true") || val.value() == QStringLiteral("1");
    }

    int getInt(const QString &key, int defaultValue) override
    {
        auto val = get(key);
        if (!val.has_value()) {
            return defaultValue;
        }
        bool ok = false;
        int result = val.value().toInt(&ok);
        return ok ? result : defaultValue;
    }

    QVariantMap m_settings;
};

/**
 * @brief Mock IPlayerStateRepository for testing
 */
class MockPlayerStateRepository : public IPlayerStateRepository {
public:
    void save(const PersistedPlayerState &state) override { m_savedState = state; }

    std::optional<PersistedPlayerState> load() override { return m_savedState; }

    void clear() override { m_savedState.reset(); }

    std::optional<PersistedPlayerState> m_savedState;
};

} // namespace

class TestPlaybackController : public QObject {
    Q_OBJECT

private:
    MockBackend *m_backend = nullptr;
    MockSettingsRepository *m_settingsRepo = nullptr;
    MockPlayerStateRepository *m_playerStateRepo = nullptr;
    std::unique_ptr<PlaybackController> m_controller;

    static Song makeSong(const QString &id, const QUrl &mediaUri = {})
    {
        Song s;
        s.id = id;
        s.name = QStringLiteral("Song %1").arg(id);
        s.artist = QStringLiteral("Artist");
        s.mediaUri = mediaUri;
        return s;
    }

private Q_SLOTS:
    void init()
    {
        m_backend = new MockBackend();
        m_settingsRepo = new MockSettingsRepository();
        m_playerStateRepo = new MockPlayerStateRepository();
        m_controller = std::make_unique<PlaybackController>(
            std::unique_ptr<IPlayerBackend>(m_backend),
            nullptr, // No platform plugin for basic tests
            m_playerStateRepo,
            m_settingsRepo);
    }

    void cleanup()
    {
        m_controller.reset();
    }

    // --- Construction ---

    void constructor_restoresVolumeFromSettings()
    {
        m_settingsRepo->set(QStringLiteral("player/volume"), QStringLiteral("0.75"));

        // Create a fresh controller to test constructor behavior
        auto *backend = new MockBackend();
        auto controller = std::make_unique<PlaybackController>(
            std::unique_ptr<IPlayerBackend>(backend),
            nullptr, m_playerStateRepo, m_settingsRepo);

        QCOMPARE(backend->volume(), 0.75);
    }

    void constructor_restoresMutedFromSettings()
    {
        m_settingsRepo->set(QStringLiteral("player/muted"), QStringLiteral("true"));

        auto *backend = new MockBackend();
        auto controller = std::make_unique<PlaybackController>(
            std::unique_ptr<IPlayerBackend>(backend),
            nullptr, m_playerStateRepo, m_settingsRepo);

        QVERIFY(backend->isMuted());
    }

    // --- Play with mediaUri ---

    void play_withMediaUri_loadsAndPlays()
    {
        QCoro::waitFor([](PlaybackController *ctrl) -> QCoro::Task<void> {
            Song song;
            song.id = QStringLiteral("test");
            song.mediaUri = QUrl(QStringLiteral("file:///test.mp3"));

            co_await ctrl->play(song);
        }(m_controller.get()));

        QCOMPARE(m_backend->m_loadCount, 1);
        QCOMPARE(m_backend->m_lastLoadedUrl, QUrl(QStringLiteral("file:///test.mp3")));
        QCOMPARE(m_backend->m_playCount, 1);
    }

    void play_withMediaUri_emitsCurrentSongChanged()
    {
        QSignalSpy spy(m_controller.get(), &PlaybackController::currentSongChanged);

        QCoro::waitFor([](PlaybackController *ctrl) -> QCoro::Task<void> {
            Song song;
            song.id = QStringLiteral("test");
            song.mediaUri = QUrl(QStringLiteral("file:///test.mp3"));
            co_await ctrl->play(song);
        }(m_controller.get()));

        QCOMPARE(spy.count(), 1);
    }

    // --- Pause / Resume / Stop / Seek ---

    void pause_delegatesToBackend()
    {
        m_controller->pause();
        QCOMPARE(m_backend->m_pauseCount, 1);
    }

    void resume_delegatesToBackend()
    {
        m_controller->resume();
        QCOMPARE(m_backend->m_playCount, 1);
    }

    void stop_delegatesToBackend()
    {
        m_controller->stop();
        QCOMPARE(m_backend->m_stopCount, 1);
    }

    void seek_delegatesToBackend()
    {
        m_controller->seek(5000);
        QCOMPARE(m_backend->m_lastSeekPosition, 5000);
    }

    // --- Volume ---

    void setVolume_setsOnBackendAndPersists()
    {
        m_controller->setVolume(0.5);
        QCOMPARE(m_backend->volume(), 0.5);

        auto saved = m_settingsRepo->get(QStringLiteral("player/volume"));
        QVERIFY(saved.has_value());
        QCOMPARE(saved.value(), QStringLiteral("0.50"));
    }

    void setMuted_setsOnBackendAndPersists()
    {
        m_controller->setMuted(true);
        QVERIFY(m_backend->isMuted());

        auto saved = m_settingsRepo->get(QStringLiteral("player/muted"));
        QVERIFY(saved.has_value());
        QCOMPARE(saved.value(), QStringLiteral("true"));
    }

    // --- Queue ---

    void queue_returnsPlayQueue()
    {
        auto *queue = m_controller->queue();
        QVERIFY(queue != nullptr);
        QVERIFY(queue->isEmpty());
    }

    // --- Backend signals ---

    void backendStateChanged_propagatesAsSignal()
    {
        QSignalSpy spy(m_controller.get(), &PlaybackController::playbackStateChanged);
        Q_EMIT m_backend->stateChanged(PlaybackState::Playing);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_controller->playbackState(), PlaybackState::Playing);
    }

    void backendPositionChanged_propagates()
    {
        QSignalSpy spy(m_controller.get(), &PlaybackController::positionChanged);
        Q_EMIT m_backend->positionChanged(5000);
        QCOMPARE(spy.count(), 1);
    }

    void backendDurationChanged_propagates()
    {
        QSignalSpy spy(m_controller.get(), &PlaybackController::durationChanged);
        Q_EMIT m_backend->durationChanged(300000);
        QCOMPARE(spy.count(), 1);
    }

    void backendError_propagates()
    {
        QSignalSpy spy(m_controller.get(), &PlaybackController::errorOccurred);
        Q_EMIT m_backend->errorOccurred(QStringLiteral("test error"));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_controller->playbackState(), PlaybackState::Error);
    }

    // --- Persistence ---

    void pause_savesState()
    {
        m_controller->queue()->setSongs({makeSong("A"), makeSong("B")});
        m_controller->pause();

        QVERIFY(m_playerStateRepo->m_savedState.has_value());
        QCOMPARE(m_playerStateRepo->m_savedState->playlist.size(), 2);
        QVERIFY(m_playerStateRepo->m_savedState->shouldResumePlayback);
    }

    // --- Backend name ---

    void backendName_returnsMock()
    {
        QCOMPARE(m_controller->backendName(), QStringLiteral("Mock"));
    }
};

QTEST_MAIN(TestPlaybackController)
#include "TestPlaybackController.moc"
