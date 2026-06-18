/// @file PlayHistoryRepository.h
/// @brief SQLite-backed play history repository

#ifndef QERIPLAYERQT_PLAYHISTORYREPOSITORY_H
#define QERIPLAYERQT_PLAYHISTORYREPOSITORY_H

#include "repo/IPlayHistoryRepository.h"

namespace QeriPlayerQt {

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

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_PLAYHISTORYREPOSITORY_H
