/// @file ISettingsRepository.h
/// @brief Interface for settings persistence operations

#ifndef NERIPLAYERQT_ISETTINGSREPOSITORY_H
#define NERIPLAYERQT_ISETTINGSREPOSITORY_H

#include <QString>
#include <QVariantMap>

#include <optional>

namespace NeriPlayerQt {

/**
 * @brief Abstract interface for settings operations on the settings table
 */
class ISettingsRepository {
public:
    virtual ~ISettingsRepository() = default;

    /**
     * @brief Get a setting value by key
     * @return Value if key exists, empty optional otherwise
     */
    virtual std::optional<QString> get(const QString &key) = 0;

    /**
     * @brief Set a setting value (upsert)
     */
    virtual void set(const QString &key, const QString &value) = 0;

    /**
     * @brief Delete a setting by key (no-op if key doesn't exist)
     */
    virtual void remove(const QString &key) = 0;

    /**
     * @brief Get all settings as key→value map
     */
    virtual QVariantMap getAll() = 0;

    /**
     * @brief Get a setting as a boolean
     * @param defaultValue Fallback if key doesn't exist or can't be converted
     */
    virtual bool getBool(const QString &key, bool defaultValue = false) = 0;

    /**
     * @brief Get a setting as an integer
     * @param defaultValue Fallback if key doesn't exist or can't be converted
     */
    virtual int getInt(const QString &key, int defaultValue = 0) = 0;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_ISETTINGSREPOSITORY_H
