/// @file SettingsRepository.cpp
/// @brief SQLite-backed settings repository implementation

#include "repo/SettingsRepository.h"

#include "core/database/DatabaseManager.h"

namespace QeriPlayerQt {

SettingsRepository::SettingsRepository(DatabaseManager *db)
    : m_db(db)
{
}

std::optional<QString> SettingsRepository::get(const QString &key)
{
    auto rows = m_db->exec("SELECT value FROM settings WHERE key = ?", {key});
    if (rows.isEmpty())
        return std::nullopt;
    return rows[0][0].toString();
}

void SettingsRepository::set(const QString &key, const QString &value)
{
    m_db->exec("INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?)", {key, value});
}

void SettingsRepository::remove(const QString &key)
{
    m_db->exec("DELETE FROM settings WHERE key = ?", {key});
}

QVariantMap SettingsRepository::getAll()
{
    auto rows = m_db->exec("SELECT key, value FROM settings");
    QVariantMap map;
    for (const auto &row : rows) {
        map[row[0].toString()] = row[1].toString();
    }
    return map;
}

bool SettingsRepository::getBool(const QString &key, bool defaultValue)
{
    auto val = get(key);
    if (!val.has_value())
        return defaultValue;
    QString lower = val->toLower();
    if (lower == "true" || lower == "1")
        return true;
    if (lower == "false" || lower == "0")
        return false;
    return defaultValue;
}

int SettingsRepository::getInt(const QString &key, int defaultValue)
{
    auto val = get(key);
    if (!val.has_value())
        return defaultValue;
    bool ok = false;
    int result = val->toInt(&ok);
    return ok ? result : defaultValue;
}

} // namespace QeriPlayerQt
