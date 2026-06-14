/// @file CryptoUtils.cpp
/// @brief Cryptographic utilities implementation using OpenSSL
/// @date 2024-01-15

#include "core/crypto/CryptoUtils.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <array>
#include <iomanip>
#include <sstream>

namespace NeriPlayerQt {

QByteArray CryptoUtils::generateKey()
{
    QByteArray key(32, '\0');
    if (RAND_bytes(reinterpret_cast<unsigned char *>(key.data()), 32) != 1) {
        throw CryptoError("Failed to generate random key");
    }
    return key;
}

QByteArray CryptoUtils::sha256(const QByteArray &data)
{
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;

    if (EVP_Digest(data.constData(), data.size(), hash, &length, EVP_sha256(), nullptr) != 1) {
        throw CryptoError("SHA-256 computation failed");
    }

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < length; ++i) {
        oss << std::setw(2) << static_cast<int>(hash[i]);
    }

    return QByteArray::fromStdString(oss.str());
}

} // namespace NeriPlayerQt
