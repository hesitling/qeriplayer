/// @file Decryptor.h
/// @brief AES-256-GCM decryption
/// @date 2024-01-15

#ifndef NERIPLAYERQT_DECRYPTOR_H
#define NERIPLAYERQT_DECRYPTOR_H

#include <QByteArray>

namespace NeriPlayerQt {

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

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_DECRYPTOR_H
