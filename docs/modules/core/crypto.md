# Crypto Module (crypto/)

## 1. Overview

The crypto module provides data encryption, secure storage, hash computation, and random number generation capabilities.

## 2. Directory Structure

```
src/core/crypto/
├── CryptoManager.h/.cpp    # Crypto manager
├── SecureStorage.h/.cpp    # Secure storage
└── HashUtils.h/.cpp        # Hash utilities
```

## 3. Main Class Design

### 3.1 CryptoManager

Crypto manager.

```cpp
class CryptoManager {
public:
    // ==================== Symmetric Encryption ====================
    
    // AES encryption
    static QByteArray encryptAes(const QByteArray &data, 
                                 const QByteArray &key,
                                 const QByteArray &iv = {});
    static QByteArray decryptAes(const QByteArray &data, 
                                 const QByteArray &key,
                                 const QByteArray &iv = {});
    
    // AES-GCM encryption (authenticated)
    static QByteArray encryptAesGcm(const QByteArray &data,
                                    const QByteArray &key,
                                    const QByteArray &iv,
                                    QByteArray &tag);
    static QByteArray decryptAesGcm(const QByteArray &data,
                                    const QByteArray &key,
                                    const QByteArray &iv,
                                    const QByteArray &tag);
    
    // ==================== Asymmetric Encryption ====================
    
    // RSA encryption
    static QByteArray encryptRsa(const QByteArray &data,
                                 const QByteArray &publicKey);
    static QByteArray decryptRsa(const QByteArray &data,
                                 const QByteArray &privateKey);
    
    // RSA signing
    static QByteArray signRsa(const QByteArray &data,
                              const QByteArray &privateKey);
    static bool verifyRsa(const QByteArray &data,
                          const QByteArray &signature,
                          const QByteArray &publicKey);
    
    // ==================== Encoding ====================
    
    // Base64 encoding
    static QString toBase64(const QByteArray &data);
    static QByteArray fromBase64(const QString &str);
    static QString toBase64Url(const QByteArray &data);
    static QByteArray fromBase64Url(const QString &str);
    
    // URL encoding
    static QString urlEncode(const QString &str);
    static QString urlDecode(const QString &str);
    
    // Hex encoding
    static QString toHex(const QByteArray &data);
    static QByteArray fromHex(const QString &str);
    
    // ==================== Hashing ====================
    
    static QString md5(const QByteArray &data);
    static QString sha1(const QByteArray &data);
    static QString sha256(const QByteArray &data);
    static QString sha512(const QByteArray &data);
    
    // File hashing
    static QString fileMd5(const QString &filePath);
    static QString fileSha256(const QString &filePath);
    
    // ==================== HMAC ====================
    
    static QString hmacSha256(const QByteArray &data, 
                              const QByteArray &key);
    static QString hmacSha512(const QByteArray &data,
                              const QByteArray &key);
    
    // ==================== Random Numbers ====================
    
    static QByteArray randomBytes(int size);
    static QString randomHex(int size);
    static int randomInt(int min, int max);
    static double randomDouble(double min, double max);
    
    // UUID
    static QString generateUuid();
};
```

### 3.2 SecureStorage

Secure storage that uses the system keyring to store sensitive information.

```cpp
class SecureStorage {
public:
    // Store credentials
    static bool store(const QString &key, const QString &value);
    
    // Retrieve credentials
    static QString retrieve(const QString &key);
    
    // Delete credentials
    static bool remove(const QString &key);
    
    // Check existence
    static bool contains(const QString &key);
    
    // Clear all
    static void clear();
    
    // Get all keys
    static QStringList keys();
    
private:
    // Platform-specific implementations
#ifdef Q_OS_WIN
    // Windows Credential Manager
    static bool storeWindows(const QString &key, const QString &value);
    static QString retrieveWindows(const QString &key);
    static bool removeWindows(const QString &key);
#elif defined(Q_OS_MACOS)
    // macOS Keychain
    static bool storeMacOS(const QString &key, const QString &value);
    static QString retrieveMacOS(const QString &key);
    static bool removeMacOS(const QString &key);
#elif defined(Q_OS_LINUX)
    // Linux Secret Service (libsecret)
    static bool storeLinux(const QString &key, const QString &value);
    static QString retrieveLinux(const QString &key);
    static bool removeLinux(const QString &key);
#endif
};
```

### 3.3 HashUtils

Hash utility class.

```cpp
class HashUtils {
public:
    // String hashing
    static uint32_t hash32(const QString &str);
    static uint64_t hash64(const QString &str);
    
    // Byte array hashing
    static uint32_t hash32(const QByteArray &data);
    static uint64_t hash64(const QByteArray &data);
    
    // FNV-1a hashing
    static uint32_t fnv1a32(const QByteArray &data);
    static uint64_t fnv1a64(const QByteArray &data);
    
    // MurmurHash
    static uint32_t murmurHash3(const QByteArray &data, uint32_t seed = 0);
};
```

## 4. Usage Examples

### 4.1 Data Encryption

```cpp
// AES encryption
QByteArray data = "Sensitive data";
QByteArray key = CryptoManager::randomBytes(32); // 256-bit key
QByteArray iv = CryptoManager::randomBytes(16);  // 128-bit IV

QByteArray encrypted = CryptoManager::encryptAes(data, key, iv);
QByteArray decrypted = CryptoManager::decryptAes(encrypted, key, iv);

// AES-GCM encryption (authenticated)
QByteArray tag;
QByteArray encryptedGcm = CryptoManager::encryptAesGcm(data, key, iv, tag);
QByteArray decryptedGcm = CryptoManager::decryptAesGcm(encryptedGcm, key, iv, tag);
```

### 4.2 Secure Storage

```cpp
// Store API tokens
SecureStorage::store("netease_token", "abc123...");
SecureStorage::store("bilibili_cookie", "cookie_data...");

// Retrieve token
QString token = SecureStorage::retrieve("netease_token");
if (token.isEmpty()) {
    // Need to re-authenticate
}

// Delete credentials
SecureStorage::remove("bilibili_cookie");
```

### 4.3 Hash Computation

```cpp
// Compute file hash
QString fileHash = CryptoManager::fileSha256("song.mp3");

// Compute string hash
QString dataHash = CryptoManager::sha256("some data");

// HMAC signature
QString signature = CryptoManager::hmacSha256(
    "data to sign", 
    "secret_key"
);
```

### 4.4 Random Number Generation

```cpp
// Generate random key
QByteArray key = CryptoManager::randomBytes(32);

// Generate random ID
QString id = CryptoManager::generateUuid();

// Generate random number
int random = CryptoManager::randomInt(1, 100);
```

## 5. Platform Implementations

### 5.1 Windows

Uses Windows Credential Manager for credential storage.

```cpp
#ifdef Q_OS_WIN
#include <wincred.h>

bool SecureStorage::storeWindows(const QString &key, const QString &value) {
    CREDENTIALW cred = {};
    cred.Type = CRED_TYPE_GENERIC;
    cred.TargetName = (LPWSTR)key.utf16();
    cred.CredentialBlobSize = value.size();
    cred.CredentialBlob = (LPBYTE)value.utf16();
    cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
    
    return CredWriteW(&cred, 0);
}
#endif
```

### 5.2 macOS

Uses macOS Keychain for credential storage.

```cpp
#ifdef Q_OS_MACOS
#include <Security/Security.h>

bool SecureStorage::storeMacOS(const QString &key, const QString &value) {
    QByteArray keyData = key.toUtf8();
    QByteArray valueData = value.toUtf8();
    
    SecKeychainAddGenericPassword(
        NULL,
        keyData.size(), keyData.constData(),
        valueData.size(), valueData.constData(),
        NULL
    );
    
    return true;
}
#endif
```

### 5.3 Linux

Uses libsecret for credential storage.

```cpp
#ifdef Q_OS_LINUX
#include <libsecret/secret.h>

bool SecureStorage::storeLinux(const QString &key, const QString &value) {
    GError *error = NULL;
    
    secret_password_store_sync(
        &schema, SECRET_COLLECTION_DEFAULT,
        key.toUtf8().constData(),
        value.toUtf8().constData(),
        NULL, &error,
        NULL
    );
    
    return error == NULL;
}
#endif
```

## 6. Testing

```cpp
class CryptoManagerTest : public QObject {
    Q_OBJECT
private slots:
    void testAesEncryptDecrypt();
    void testAesGcmEncryptDecrypt();
    void testRsaEncryptDecrypt();
    void testRsaSignVerify();
    void testBase64();
    void testHash();
    void testRandom();
};

class SecureStorageTest : public QObject {
    Q_OBJECT
private slots:
    void testStoreRetrieve();
    void testRemove();
    void testContains();
};
```

## 7. Summary

The crypto module provides complete security support:
- **CryptoManager**: Symmetric/asymmetric encryption, hash computation, random number generation
- **SecureStorage**: System keyring integration, sensitive information storage
- **HashUtils**: High-performance hash algorithms
