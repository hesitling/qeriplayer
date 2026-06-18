/// @file TestQtMultimediaBackend.cpp
/// @brief Integration tests for QtMultimediaBackend

#include "player/backends/QtMultimediaBackend.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace QeriPlayerQt;

class TestQtMultimediaBackend : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void init()
    {
        // Each test gets a fresh backend
    }

    // --- Basic construction ---

    void construction_initialStateIsStopped()
    {
        QtMultimediaBackend backend;
        QCOMPARE(backend.state(), PlaybackState::Stopped);
    }

    void construction_backendName()
    {
        QtMultimediaBackend backend;
        QCOMPARE(backend.backendName(), QStringLiteral("Qt Multimedia"));
    }

    // --- Volume ---

    void setVolume_setsVolume()
    {
        QtMultimediaBackend backend;
        backend.setVolume(0.5);
        QCOMPARE(backend.volume(), 0.5);
    }

    void setVolume_clampsToRange()
    {
        QtMultimediaBackend backend;
        backend.setVolume(1.5);
        QCOMPARE(backend.volume(), 1.0);

        backend.setVolume(-0.5);
        QCOMPARE(backend.volume(), 0.0);
    }

    // --- Mute ---

    void setMuted_toggles()
    {
        QtMultimediaBackend backend;
        QVERIFY(!backend.isMuted());
        backend.setMuted(true);
        QVERIFY(backend.isMuted());
        backend.setMuted(false);
        QVERIFY(!backend.isMuted());
    }

    // --- Signals ---

    void stateChanged_onStop_doesNotCrash()
    {
        QtMultimediaBackend backend;
        QSignalSpy spy(&backend, &IPlayerBackend::stateChanged);
        backend.stop();
        // Stop on already-stopped player may or may not emit signal
        // Just verify it doesn't crash
        Q_UNUSED(spy);
    }

    // --- Seek ---

    void seek_doesNotCrash()
    {
        QtMultimediaBackend backend;
        backend.seek(5000);
        // Position may not update immediately without media loaded
        // Just verify it doesn't crash
    }
};

QTEST_MAIN(TestQtMultimediaBackend)
#include "TestQtMultimediaBackend.moc"
