/// @file SongRepository.h
/// @brief SQLite-backed song repository

#ifndef QERIPLAYERQT_SONGREPOSITORY_H
#define QERIPLAYERQT_SONGREPOSITORY_H

#include "repo/ISongRepository.h"

#include <optional>

namespace QeriPlayerQt {

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

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_SONGREPOSITORY_H
