/// @file NeteaseCrypto.cpp
/// @brief NetEase WeAPI encryption implementation

#include "api/netease/NeteaseCrypto.h"

#include <QCryptographicHash>
#include <QRandomGenerator>

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace QeriPlayerQt {

// NetEase WeAPI constants
const QByteArray NeteaseCrypto::AES_KEY = "0CoJUm6Qyw8W8jud";
const QByteArray NeteaseCrypto::AES_IV = "0102030405060708";
const QByteArray NeteaseCrypto::RSA_PUBLIC_MODULUS = "00e0b509f6259df8642dbc35662901477df22677ec152b5ff68ace615bb7"
                                                     "b725152b3ab17a876aea8a5aa76d2e417629ec4ee341f56135fccf695280"
                                                     "104e0312ecbda92557c93870114af6c9d05c4f7f0c3685b7a46bee255932"
                                                     "575cce10b424d813cfe4875d3e82047b97ddef52741d546b8e289dc6935b"
                                                     "3ece0462db0a22b8e7";
const QByteArray NeteaseCrypto::RSA_PUBLIC_EXPONENT = "010001";

// EAPI constants
const QByteArray NeteaseCrypto::EAPI_KEY = "e82ckenh8dichen8";
const QByteArray NeteaseCrypto::EAPI_SALT_FORMAT = "nobody%1use%2md5forencrypt";
const QByteArray NeteaseCrypto::EAPI_URL_FORMAT = "%1-36cd479b6b5-%2-36cd479b6b5-%3";

QByteArray NeteaseCrypto::aesCbcEncrypt(const QByteArray &data, const QByteArray &key, const QByteArray &iv)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP cipher context");
    }

    QByteArray encrypted;
    encrypted.resize(data.size() + EVP_MAX_BLOCK_LENGTH);
    int outLen = 0;
    int totalLen = 0;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, reinterpret_cast<const unsigned char *>(key.constData()),
                           reinterpret_cast<const unsigned char *>(iv.constData()))
        != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("AES encrypt init failed");
    }

    EVP_CIPHER_CTX_set_padding(ctx, 1); // PKCS7 padding

    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char *>(encrypted.data()), &outLen,
                          reinterpret_cast<const unsigned char *>(data.constData()), data.size())
        != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("AES encrypt update failed");
    }
    totalLen = outLen;

    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(encrypted.data()) + totalLen, &outLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("AES encrypt final failed");
    }
    totalLen += outLen;

    EVP_CIPHER_CTX_free(ctx);
    encrypted.resize(totalLen);
    return encrypted;
}

QByteArray NeteaseCrypto::rsaEncrypt(const QByteArray &data)
{
    // Reverse the data
    QByteArray reversed = data;
    std::reverse(reversed.begin(), reversed.end());

    // Convert hex strings to BIGNUMs
    BIGNUM *modulus = nullptr;
    BIGNUM *exponent = nullptr;
    BN_hex2bn(&modulus, RSA_PUBLIC_MODULUS.constData());
    BN_hex2bn(&exponent, RSA_PUBLIC_EXPONENT.constData());

    if (!modulus || !exponent) {
        BN_free(modulus);
        BN_free(exponent);
        throw std::runtime_error("Failed to parse RSA parameters");
    }

    // RSA public key operation: result = data^exponent mod modulus
    BIGNUM *dataBn = BN_new();
    BIGNUM *result = BN_new();
    BN_CTX *ctx = BN_CTX_new();

    if (!dataBn || !result || !ctx) {
        BN_free(dataBn);
        BN_free(result);
        BN_CTX_free(ctx);
        BN_free(modulus);
        BN_free(exponent);
        throw std::runtime_error("Failed to allocate BIGNUM/CTX for RSA");
    }

    // Convert reversed bytes to BIGNUM (big-endian)
    if (!BN_bin2bn(reinterpret_cast<const unsigned char *>(reversed.constData()), reversed.size(), dataBn)) {
        BN_free(dataBn);
        BN_free(result);
        BN_CTX_free(ctx);
        BN_free(modulus);
        BN_free(exponent);
        throw std::runtime_error("BN_bin2bn failed");
    }

    if (BN_mod_exp(result, dataBn, exponent, modulus, ctx) != 1) {
        BN_free(dataBn);
        BN_free(result);
        BN_CTX_free(ctx);
        BN_free(modulus);
        BN_free(exponent);
        throw std::runtime_error("BN_mod_exp failed");
    }

    // Convert result to fixed-size byte array (256 bytes, big-endian)
    QByteArray output(RSA_BLOCK_SIZE, '\0');
    int bytesWritten
        = BN_bn2bin(result, reinterpret_cast<unsigned char *>(output.data()) + RSA_BLOCK_SIZE - BN_num_bytes(result));

    BN_free(modulus);
    BN_free(exponent);
    BN_free(dataBn);
    BN_free(result);
    BN_CTX_free(ctx);

    return output;
}

NeteaseCrypto::WeapiResult NeteaseCrypto::weapiEncrypt(const QString &plaintext)
{
    // Generate random 16-char key from BASE62 (matching Android implementation)
    static const char BASE62[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QByteArray randomKey(16, '\0');
    for (int i = 0; i < 16; ++i) {
        randomKey[i] = BASE62[QRandomGenerator::global()->bounded(62)];
    }

    return weapiEncryptWithKey(plaintext, randomKey);
}

NeteaseCrypto::WeapiResult NeteaseCrypto::weapiEncryptWithKey(const QString &plaintext, const QByteArray &randomKey)
{
    // Step 1: First AES encryption with fixed key
    QByteArray firstPass = aesCbcEncrypt(plaintext.toUtf8(), AES_KEY, AES_IV);
    QByteArray firstPassBase64 = firstPass.toBase64();

    // Step 2: Second AES encryption with random key
    QByteArray secondPass = aesCbcEncrypt(firstPassBase64, randomKey, AES_IV);
    QByteArray params = secondPass.toBase64();

    // Step 3: RSA encrypt the random key
    QByteArray encSecKey = rsaEncrypt(randomKey);

    WeapiResult result;
    result.params = QString::fromLatin1(params);
    result.encSecKey = QString::fromLatin1(encSecKey.toHex());
    return result;
}

// ─── EAPI ───────────────────────────────────────────────────────────────────

QByteArray NeteaseCrypto::aesEcbEncrypt(const QByteArray &data, const QByteArray &key)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP cipher context");
    }

    QByteArray encrypted;
    encrypted.resize(data.size() + EVP_MAX_BLOCK_LENGTH);
    int outLen = 0;
    int totalLen = 0;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, reinterpret_cast<const unsigned char *>(key.constData()),
                           nullptr)
        != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("AES ECB encrypt init failed");
    }

    EVP_CIPHER_CTX_set_padding(ctx, 1); // PKCS7 padding

    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char *>(encrypted.data()), &outLen,
                          reinterpret_cast<const unsigned char *>(data.constData()), data.size())
        != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("AES ECB encrypt update failed");
    }
    totalLen = outLen;

    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(encrypted.data()) + totalLen, &outLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("AES ECB encrypt final failed");
    }
    totalLen += outLen;

    EVP_CIPHER_CTX_free(ctx);
    encrypted.resize(totalLen);
    return encrypted;
}

QString NeteaseCrypto::md5Hex(const QString &input)
{
    QByteArray hash = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Md5);
    return QString::fromLatin1(hash.toHex());
}

QString NeteaseCrypto::eapiEncrypt(const QString &url, const QString &plaintext)
{
    // Build the message: url-36cd479b6b5-data-36cd479b6b5-md5
    QString cleanUrl = url;
    cleanUrl.replace(QStringLiteral("/eapi"), QStringLiteral("/api"));

    QString salt = QString::fromLatin1(EAPI_SALT_FORMAT).arg(cleanUrl, plaintext);
    QString hash = md5Hex(salt);

    QString message = QString::fromLatin1(EAPI_URL_FORMAT).arg(cleanUrl, plaintext, hash);

    // AES-ECB encrypt and return as hex
    QByteArray encrypted = aesEcbEncrypt(message.toUtf8(), EAPI_KEY);
    return QString::fromLatin1(encrypted.toHex()).toUpper();
}

} // namespace QeriPlayerQt
