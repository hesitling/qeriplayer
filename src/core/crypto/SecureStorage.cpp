/// @file SecureStorage.cpp
/// @brief Encrypted key-value storage implementation

#include "core/crypto/SecureStorage.h"
#include "core/crypto/CryptoUtils.h"
#include "core/crypto/Decryptor.h"
#include "core/crypto/Encryptor.h"

#include "core/filesystem/FileUtils.h"

#include <QDebug>
#include <QFile>
#include <QFileDevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>

namespace NeriPlayerQt {

SecureStorage::SecureStorage(const QString &filePath)
    : m_filePath(filePath)
{
}

void SecureStorage::set(const QString &key, const QString &value)
{
    QMutexLocker lock(&m_mutex);
    if (!m_loaded) {
        load();
    }

    QByteArray encryptedValue = Encryptor::encrypt(value.toUtf8(), m_masterKey);
    m_encryptedData[key] = encryptedValue;
    save();
}

std::optional<QString> SecureStorage::get(const QString &key) const
{
    QMutexLocker lock(&m_mutex);
    if (!m_loaded) {
        load();
    }

    auto it = m_encryptedData.find(key);
    if (it == m_encryptedData.end()) {
        return std::nullopt;
    }

    try {
        QByteArray decrypted = Decryptor::decrypt(it.value(), m_masterKey);
        return QString::fromUtf8(decrypted);
    } catch (const CryptoError &ex) {
        qWarning() << "SecureStorage: decrypt failed for key" << key << ":" << ex.what();
        return std::nullopt;
    }
}

void SecureStorage::remove(const QString &key)
{
    QMutexLocker lock(&m_mutex);
    if (!m_loaded) {
        load();
    }

    m_encryptedData.remove(key);
    save();
}

bool SecureStorage::contains(const QString &key) const
{
    QMutexLocker lock(&m_mutex);
    if (!m_loaded) {
        load();
    }
    return m_encryptedData.contains(key);
}

void SecureStorage::load() const
{
    m_masterKey = deriveMasterKey();
    m_loaded = true;

    QFile file(m_filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray raw = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(raw);
    if (!doc.isObject()) {
        return;
    }

    QJsonObject root = doc.object();
    for (auto it = root.begin(); it != root.end(); ++it) {
        QByteArray encryptedBytes = QByteArray::fromBase64(it.value().toString().toUtf8());
        m_encryptedData[it.key()] = encryptedBytes;
    }
}

void SecureStorage::save() const
{
    QJsonObject root;
    for (auto it = m_encryptedData.begin(); it != m_encryptedData.end(); ++it) {
        root[it.key()] = QString::fromUtf8(it.value().toBase64());
    }

    QJsonDocument doc(root);
    if (!FileUtils::writeFile(m_filePath, doc.toJson(QJsonDocument::Compact))) {
        throw CryptoError("Failed to save secure storage: " + FileUtils::lastError().toStdString());
    }
}

QByteArray SecureStorage::deriveMasterKey() const
{
    // Use a per-machine random secret stored alongside the data
    QString secretPath = m_filePath + QStringLiteral(".key");
    QFile secretFile(secretPath);

    if (secretFile.exists() && secretFile.open(QIODevice::ReadOnly)) {
        QByteArray key = secretFile.readAll();
        secretFile.close();
        if (key.size() >= 32) {
            return key.left(32);
        }
    }

    // Generate new random key
    QByteArray key = CryptoUtils::generateKey();
    if (!secretFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        throw CryptoError("Failed to persist master key: " + secretFile.errorString().toStdString());
    }
    // Set permissions before writing to minimize window of exposure
    secretFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    qint64 written = secretFile.write(key);
    secretFile.close();
    if (written != key.size()) {
        throw CryptoError("Failed to write complete master key");
    }
    return key;
}

} // namespace NeriPlayerQt
