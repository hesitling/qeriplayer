/// @file QtMultimediaBackend.h
/// @brief Qt6 Multimedia implementation of IPlayerBackend

#ifndef NERIPLAYERQT_QTMULTIMEDIABACKEND_H
#define NERIPLAYERQT_QTMULTIMEDIABACKEND_H

#include "player/IPlayerBackend.h"

#include <QAudioOutput>
#include <QMediaPlayer>

#include <memory>

namespace NeriPlayerQt {

/**
 * @brief Qt6 Multimedia implementation of IPlayerBackend
 *
 * Uses QMediaPlayer for audio decoding and QAudioOutput for volume/device control.
 * Default backend with zero external dependencies.
 */
class QtMultimediaBackend : public IPlayerBackend {
    Q_OBJECT

public:
    explicit QtMultimediaBackend(QObject *parent = nullptr);
    ~QtMultimediaBackend() override;

    QCoro::Task<void> load(const QUrl &url) override;
    void play() override;
    void pause() override;
    void stop() override;
    void seek(qint64 positionMs) override;

    [[nodiscard]] PlaybackState state() const override;
    [[nodiscard]] qint64 positionMs() const override;
    [[nodiscard]] qint64 durationMs() const override;
    [[nodiscard]] bool isSeekable() const override;

    void setVolume(double normalized) override;
    [[nodiscard]] double volume() const override;
    void setMuted(bool muted) override;
    [[nodiscard]] bool isMuted() const override;

    [[nodiscard]] QString backendName() const override;

private:
    void connectSignals();
    void updateCachedState();

    std::unique_ptr<QMediaPlayer> m_player;
    std::unique_ptr<QAudioOutput> m_audioOutput;
    PlaybackState m_cachedState = PlaybackState::Stopped;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_QTMULTIMEDIABACKEND_H
