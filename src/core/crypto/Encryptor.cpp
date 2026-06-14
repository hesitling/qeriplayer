/// @file Encryptor.cpp
/// @brief AES-256-GCM encryption implementation
/// @date 2024-01-15

#include "core/crypto/Encryptor.h"
#include "core/crypto/CryptoUtils.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

namespace NeriPlayerQt {

static constexpr int NONCE_SIZE = 12;
static constexpr int TAG_SIZE = 16;

QByteArray Encryptor::encrypt(const QByteArray &plaintext, const QByteArray &key)
{
    if (key.size() != 32) {
        throw CryptoError("Key must be 32 bytes");
    }

    // Generate random nonce
    unsigned char nonce[NONCE_SIZE];
    if (RAND_bytes(nonce, NONCE_SIZE) != 1) {
        throw CryptoError("Failed to generate nonce");
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw CryptoError("Failed to create cipher context");
    }

    // Initialize encryption
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw CryptoError("Failed to initialize AES-256-GCM");
    }

    // Set nonce length
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, NONCE_SIZE, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw CryptoError("Failed to set nonce length");
    }

    // Set nonce
    if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, reinterpret_cast<const unsigned char *>(key.constData()), nonce)
        != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw CryptoError("Failed to set key and nonce");
    }

    // Encrypt
    QByteArray ciphertext(plaintext.size(), '\0');
    int outLen = 0;
    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char *>(ciphertext.data()), &outLen,
                          reinterpret_cast<const unsigned char *>(plaintext.constData()), plaintext.size())
        != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw CryptoError("Encryption failed");
    }

    int finalLen = 0;
    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(ciphertext.data()) + outLen, &finalLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw CryptoError("Encryption finalization failed");
    }

    // Get auth tag
    unsigned char tag[TAG_SIZE];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw CryptoError("Failed to get auth tag");
    }

    EVP_CIPHER_CTX_free(ctx);

    // Resize to actual encrypted length
    ciphertext.resize(outLen + finalLen);

    // Build output: [nonce][ciphertext][tag]
    QByteArray result;
    result.reserve(NONCE_SIZE + ciphertext.size() + TAG_SIZE);
    result.append(reinterpret_cast<const char *>(nonce), NONCE_SIZE);
    result.append(ciphertext);
    result.append(reinterpret_cast<const char *>(tag), TAG_SIZE);

    return result;
}

} // namespace NeriPlayerQt
