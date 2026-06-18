/// @file SecureStorage.h
/// @brief Encrypted key-value storage on disk

#ifndef QERIPLAYERQT_SECURESTORAGE_H
#define QERIPLAYERQT_SECURESTORAGE_H

#include <QByteArray>
#include <QHash>
#include <QMutex>
#include <QString>

#include <optional>

namespace QeriPlayerQt {

/**
 * @brief Stores key-value pairs encrypted on disk using AES-256-GCM
 *
 * The master encryption key is derived from a machine-specific value
 * and stored alongside the encrypted data.
 */
class SecureStorage {
public:
    /**
     * @brief Create or open a secure storage file
     * @param filePath Path to the storage file
     */
    explicit SecureStorage(const QString &filePath);

    /**
     * @brief Store an encrypted value
     * @param key Key name
     * @param value Value to encrypt and store
     */
    void set(const QString &key, const QString &value);

    /**
     * @brief Retrieve and decrypt a value
     * @param key Key name
     * @return Decrypted value, or nullopt if not found or decryption failed
     */
    std::optional<QString> get(const QString &key) const;

    /**
     * @brief Remove a stored value
     * @param key Key name
     */
    void remove(const QString &key);

    /**
     * @brief Check if a key exists
     */
    bool contains(const QString &key) const;

private:
    void load() const;
    void save() const;
    QByteArray deriveMasterKey() const;

    QString m_filePath;
    mutable QMutex m_mutex;
    mutable QHash<QString, QByteArray> m_encryptedData; // key -> encrypted value bytes
    mutable QByteArray m_masterKey;
    mutable bool m_loaded = false;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_SECURESTORAGE_H
