/// @file PlayHistoryRepository.h
/// @brief SQLite-backed play history repository

#ifndef NERIPLAYERQT_PLAYHISTORYREPOSITORY_H
#define NERIPLAYERQT_PLAYHISTORYREPOSITORY_H

#include "repo/IPlayHistoryRepository.h"

namespace NeriPlayerQt {

class DatabaseManager;

/**
 * @brief SQLite implementation of IPlayHistoryRepository
 */
class PlayHistoryRepository : public IPlayHistoryRepository {
public:
    explicit PlayHistoryRepository(DatabaseManager *db);

    void record(const QString &songId) override;
    QVector<Song> recent(int limit = 50) override;
    void clear() override;
    void remove(const QStringList &songIds) override;
    int playCount(const QString &songId) override;

private:
    DatabaseManager *m_db;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_PLAYHISTORYREPOSITORY_H
