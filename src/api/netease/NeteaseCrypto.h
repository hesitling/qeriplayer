/// @file NeteaseCrypto.h
/// @brief NetEase WeAPI encryption helpers

#ifndef QERIPLAYERQT_NETEASECRYPTO_H
#define QERIPLAYERQT_NETEASECRYPTO_H

#include <QByteArray>
#include <QString>

namespace QeriPlayerQt {

/**
 * @brief NetEase WeAPI encryption
 *
 * Implements the AES-128-CBC + RSA encryption used by the NetEase web client.
 * WeAPI encryption is one-way (client → server) — responses arrive over HTTPS
 * and require no client-side decryption.
 */
class NeteaseCrypto {
public:
    /**
     * @brief Encrypt plaintext using the WeAPI protocol
     * @param plaintext The text to encrypt (typically JSON)
     * @return Pair of (params, encSecKey) for POST body
     */
    struct WeapiResult {
        QString params;
        QString encSecKey;
    };

    static WeapiResult weapiEncrypt(const QString &plaintext);

    /**
     * @brief Encrypt plaintext with a fixed key for deterministic testing
     *
     * Uses the provided randomKey instead of generating one.
     * Only for unit tests — normal code should use weapiEncrypt().
     */
    static WeapiResult weapiEncryptWithKey(const QString &plaintext, const QByteArray &randomKey);

    /**
     * @brief Encrypt payload using the EAPI protocol
     * @param url The API URL path (e.g., "/api/login/cellphone")
     * @param plaintext JSON payload to encrypt
     * @return Hex-encoded params string
     */
    static QString eapiEncrypt(const QString &url, const QString &plaintext);

    /**
     * @brief Compute MD5 hex digest
     */
    static QString md5Hex(const QString &input);

private:
    static QByteArray aesCbcEncrypt(const QByteArray &data, const QByteArray &key, const QByteArray &iv);
    static QByteArray aesEcbEncrypt(const QByteArray &data, const QByteArray &key);
    static QByteArray rsaEncrypt(const QByteArray &data);

    // WeAPI constants
    static const QByteArray AES_KEY;
    static const QByteArray AES_IV;
    static const QByteArray RSA_PUBLIC_MODULUS;
    static const QByteArray RSA_PUBLIC_EXPONENT;
    static const int RSA_BLOCK_SIZE = 128; // 1024-bit RSA key = 128 bytes

    // EAPI constants
    static const QByteArray EAPI_KEY;
    static const QByteArray EAPI_SALT_FORMAT;
    static const QByteArray EAPI_URL_FORMAT;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_NETEASECRYPTO_H
