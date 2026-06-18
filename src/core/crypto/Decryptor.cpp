/// @file Decryptor.cpp
/// @brief AES-256-GCM decryption implementation

#include "core/crypto/Decryptor.h"
#include "core/crypto/CryptoUtils.h"

#include <openssl/evp.h>

namespace QeriPlayerQt {

static constexpr int NONCE_SIZE = 12;
static constexpr int TAG_SIZE = 16;

QByteArray Decryptor::decrypt(const QByteArray &ciphertext, const QByteArray &key)
{
    if (key.size() != 32) {
        throw CryptoError("Key must be 32 bytes");
    }

    // Minimum size: nonce + tag (no ciphertext is valid)
    if (ciphertext.size() < NONCE_SIZE + TAG_SIZE) {
        throw CryptoError("Ciphertext too short");
    }

    // Extract nonce, encrypted data, and tag
    const unsigned char *nonce = reinterpret_cast<const unsigned char *>(ciphertext.constData());
    const unsigned char *encData = nonce + NONCE_SIZE;
    int encLen = ciphertext.size() - NONCE_SIZE - TAG_SIZE;
    const unsigned char *tag
        = reinterpret_cast<const unsigned char *>(ciphertext.constData()) + ciphertext.size() - TAG_SIZE;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw CryptoError("Failed to create cipher context");
    }

    // Initialize decryption
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw CryptoError("Failed to initialize AES-256-GCM");
    }

    // Set nonce length
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, NONCE_SIZE, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw CryptoError("Failed to set nonce length");
    }

    // Set nonce and key
    if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, reinterpret_cast<const unsigned char *>(key.constData()), nonce)
        != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw CryptoError("Failed to set key and nonce");
    }

    // Decrypt
    QByteArray plaintext(encLen, '\0');
    int outLen = 0;
    if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(plaintext.data()), &outLen, encData, encLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw CryptoError("Decryption failed");
    }

    // Set expected auth tag
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, const_cast<unsigned char *>(tag)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw CryptoError("Failed to set auth tag");
    }

    // Finalize and verify tag
    int finalLen = 0;
    if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(plaintext.data()) + outLen, &finalLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw CryptoError("Authentication failed — ciphertext may be tampered");
    }

    EVP_CIPHER_CTX_free(ctx);

    plaintext.resize(outLen + finalLen);
    return plaintext;
}

} // namespace QeriPlayerQt
