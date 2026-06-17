/// @file SongRepository.h
/// @brief SQLite-backed song repository

#ifndef NERIPLAYERQT_SONGREPOSITORY_H
#define NERIPLAYERQT_SONGREPOSITORY_H

#include "repo/ISongRepository.h"

#include <optional>

namespace NeriPlayerQt {

class DatabaseManager;

/**
 * @brief SQLite implementation of ISongRepository
 */
class SongRepository : public ISongRepository {
public:
    explicit SongRepository(DatabaseManager *db);

    std::optional<Song> findById(const QString &id) override;
    QVector<Song> findByIds(const QStringList &ids) override;
    void save(const Song &song) override;
    void saveBatch(const QVector<Song> &songs) override;
    void remove(const QString &id) override;
    bool exists(const QString &id) override;
    QVector<Song> findByPlatform(MusicPlatform platform) override;
    QVector<Song> search(const QString &query, int limit = 50) override;

private:
    DatabaseManager *m_db;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_SONGREPOSITORY_H
