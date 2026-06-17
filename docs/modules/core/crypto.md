# Crypto Module (core/crypto/)

## Overview

The crypto module provides AES-256-GCM encryption/decryption, cryptographic key generation, SHA-256 hashing, and encrypted file-based key-value storage for secrets (cookies, tokens).

## Source Files

```
src/core/crypto/
├── CryptoUtils.h / .cpp    # Key generation, SHA-256, CryptoError
├── Encryptor.h / .cpp      # AES-256-GCM encryption
├── Decryptor.h / .cpp      # AES-256-GCM decryption
└── SecureStorage.h / .cpp  # Encrypted key-value storage on disk
```

## CryptoUtils

Utility functions for key generation and hashing.

```cpp
class CryptoUtils {
public:
    static QByteArray generateKey();          // Random 32-byte key
    static QByteArray sha256(const QByteArray &data); // SHA-256 hex string
};

class CryptoError : public std::runtime_error { ... };
```

## Encryptor

AES-256-GCM encryption. Output format: `[12-byte nonce][ciphertext][16-byte auth tag]`.

```cpp
class Encryptor {
public:
    static QByteArray encrypt(const QByteArray &plaintext, const QByteArray &key);
};
```

- A random 12-byte nonce is generated for each encryption and prepended to the output.
- The key must be 32 bytes (256 bits).

## Decryptor

AES-256-GCM decryption. Extracts the nonce, verifies the auth tag, returns plaintext.

```cpp
class Decryptor {
public:
    static QByteArray decrypt(const QByteArray &ciphertext, const QByteArray &key);
};
```

- Throws `CryptoError` if the authentication tag is invalid (wrong key or tampered data).
- Input must match the format produced by `Encryptor::encrypt()`.

## SecureStorage

Encrypted key-value storage on disk. Values are encrypted with AES-256-GCM before writing. The master key is derived from a machine-specific value and stored alongside the encrypted data.

```cpp
class SecureStorage {
public:
    explicit SecureStorage(const QString &filePath);

    void set(const QString &key, const QString &value);
    QString get(const QString &key) const;  // Empty string if not found
    void remove(const QString &key);
    bool contains(const QString &key) const;
};
```

### Design Decisions

- **File-based storage** (not OS keyring) — the master encryption key is derived from a machine-specific value and stored with the encrypted data file.
- **Thread-safe** via internal `QMutex`.
- **Lazy loading** — the storage file is read on first access.
- The storage file on disk contains no plaintext values.

## Usage

```cpp
// Generate a key and encrypt/decrypt
QByteArray key = CryptoUtils::generateKey();
QByteArray encrypted = Encryptor::encrypt("secret data", key);
QByteArray decrypted = Decryptor::decrypt(encrypted, key);

// Secure storage for cookies
SecureStorage storage(AppPaths::dataDir() + "/secrets.dat");
storage.set("netease_cookie", "MUSIC_U=xxx; __csrf=yyy");
QString cookie = storage.get("netease_cookie");

// Hashing
QByteArray hash = CryptoUtils::sha256("hello");
// hash == "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824"
```

## Testing

See `tests/core/TestCrypto.cpp`.
