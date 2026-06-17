/// @file PlaylistRepository.h
/// @brief SQLite-backed playlist repository

#ifndef NERIPLAYERQT_PLAYLISTREPOSITORY_H
#define NERIPLAYERQT_PLAYLISTREPOSITORY_H

#include "repo/IPlaylistRepository.h"

#include <QUuid>

#include <optional>

namespace NeriPlayerQt {

class DatabaseManager;

/**
 * @brief SQLite implementation of IPlaylistRepository
 */
class PlaylistRepository : public IPlaylistRepository {
public:
    explicit PlaylistRepository(DatabaseManager *db);

    QVector<PlaylistSummary> findAll() override;
    std::optional<Playlist> findById(const QString &id) override;
    Playlist create(const QString &name, MusicPlatform platform = MusicPlatform::Unknown) override;
    void updateMetadata(const QString &id, const QString &name, const QString &description,
                        const QString &coverUrl) override;
    void remove(const QString &id) override;
    bool addSong(const QString &playlistId, const QString &songId, int position = -1) override;
    void removeSong(const QString &playlistId, const QString &songId) override;
    void reorderSongs(const QString &playlistId, const QStringList &songIds) override;
    int songCount(const QString &playlistId) override;

private:
    DatabaseManager *m_db;
    void updateSongCount(const QString &playlistId);
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_PLAYLISTREPOSITORY_H
