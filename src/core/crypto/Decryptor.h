/// @file Decryptor.h
/// @brief AES-256-GCM decryption

#ifndef QERIPLAYERQT_DECRYPTOR_H
#define QERIPLAYERQT_DECRYPTOR_H

#include <QByteArray>

namespace QeriPlayerQt {

/**
 * @brief AES-256-GCM decryptor
 *
 * Expects input format: [12-byte nonce][ciphertext][16-byte auth tag]
 */
class Decryptor {
public:
    /**
     * @brief Decrypt ciphertext with AES-256-GCM
     * @param ciphertext Encrypted data with nonce and auth tag
     * @param key 32-byte encryption key
     * @return Decrypted plaintext
     * @throws CryptoError on authentication failure or invalid key
     */
    static QByteArray decrypt(const QByteArray &ciphertext, const QByteArray &key);
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_DECRYPTOR_H
