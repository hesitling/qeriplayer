/// @file SecureStorage.cpp
/// @brief Encrypted key-value storage implementation
/// @date 2024-01-15

#include "core/crypto/SecureStorage.h"
#include "core/crypto/CryptoUtils.h"
#include "core/crypto/Decryptor.h"
#include "core/crypto/Encryptor.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace NeriPlayerQt {

SecureStorage::SecureStorage(const QString &filePath)
    : m_filePath(filePath)
{
}

void SecureStorage::set(const QString &key, const QString &value)
{
    if (!m_loaded) {
        load();
    }

    QByteArray encryptedValue = Encryptor::encrypt(value.toUtf8(), m_masterKey);
    m_encryptedData[key] = encryptedValue;
    save();
}

QString SecureStorage::get(const QString &key) const
{
    if (!m_loaded) {
        load();
    }

    auto it = m_encryptedData.find(key);
    if (it == m_encryptedData.end()) {
        return { };
    }

    try {
        QByteArray decrypted = Decryptor::decrypt(it.value(), m_masterKey);
        return QString::fromUtf8(decrypted);
    } catch (const CryptoError &) {
        return { };
    }
}

void SecureStorage::remove(const QString &key)
{
    if (!m_loaded) {
        load();
    }

    m_encryptedData.remove(key);
    save();
}

bool SecureStorage::contains(const QString &key) const
{
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
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        throw CryptoError("Failed to save secure storage: " + file.errorString().toStdString());
    }
    file.write(doc.toJson(QJsonDocument::Compact));
    file.close();
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
    qint64 written = secretFile.write(key);
    secretFile.close();
    if (written != key.size()) {
        throw CryptoError("Failed to write complete master key");
    }
    return key;
}

} // namespace NeriPlayerQt
