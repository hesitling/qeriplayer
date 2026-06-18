/// @file Encryptor.h
/// @brief AES-256-GCM encryption

#ifndef QERIPLAYERQT_ENCRYPTOR_H
#define QERIPLAYERQT_ENCRYPTOR_H

#include <QByteArray>

namespace QeriPlayerQt {

/**
 * @brief AES-256-GCM encryptor
 *
 * Output format: [12-byte nonce][ciphertext][16-byte auth tag]
 */
class Encryptor {
public:
    /**
     * @brief Encrypt plaintext with AES-256-GCM
     * @param plaintext Data to encrypt
     * @param key 32-byte encryption key
     * @return Encrypted data with prepended nonce and appended auth tag
     * @throws CryptoError on failure
     */
    static QByteArray encrypt(const QByteArray &plaintext, const QByteArray &key);
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_ENCRYPTOR_H
