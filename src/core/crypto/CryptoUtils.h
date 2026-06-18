/// @file CryptoUtils.h
/// @brief Cryptographic utilities: key generation, hashing

#ifndef QERIPLAYERQT_CRYPTOUTILS_H
#define QERIPLAYERQT_CRYPTOUTILS_H

#include <QByteArray>
#include <QString>

#include <stdexcept>

namespace QeriPlayerQt {

/**
 * @brief Exception thrown on cryptographic errors
 */
class CryptoError : public std::runtime_error {
public:
    explicit CryptoError(const std::string &message)
        : std::runtime_error(message)
    {
    }
};

/**
 * @brief Cryptographic utility functions
 */
class CryptoUtils {
public:
    /**
     * @brief Generate a cryptographically random 32-byte key
     */
    static QByteArray generateKey();

    /**
     * @brief Compute SHA-256 hash as lowercase hex string
     * @param data Input data
     * @return 64-character hex string
     */
    static QByteArray sha256(const QByteArray &data);
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_CRYPTOUTILS_H
