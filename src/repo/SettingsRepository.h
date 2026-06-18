/// @file SettingsRepository.h
/// @brief SQLite-backed settings repository

#ifndef QERIPLAYERQT_SETTINGSREPOSITORY_H
#define QERIPLAYERQT_SETTINGSREPOSITORY_H

#include "repo/ISettingsRepository.h"

#include <optional>

namespace QeriPlayerQt {

class DatabaseManager;

/**
 * @brief SQLite implementation of ISettingsRepository
 */
class SettingsRepository : public ISettingsRepository {
public:
    explicit SettingsRepository(DatabaseManager *db);

    std::optional<QString> get(const QString &key) override;
    void set(const QString &key, const QString &value) override;
    void remove(const QString &key) override;
    QVariantMap getAll() override;
    bool getBool(const QString &key, bool defaultValue = false) override;
    int getInt(const QString &key, int defaultValue = 0) override;

private:
    DatabaseManager *m_db;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_SETTINGSREPOSITORY_H
