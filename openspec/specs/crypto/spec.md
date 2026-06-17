## Purpose

Defines the cryptographic utilities used by the application: AES-256-GCM encryption/decryption, secure key derivation, and encrypted file storage for secrets (cookies, tokens).

## Requirements

### Requirement: AES-256-GCM encryption
The system SHALL provide an `Encryptor` class with an `encrypt(plaintext, key)` method that returns ciphertext with an appended authentication tag. The key SHALL be 32 bytes (256 bits). A random 12-byte nonce SHALL be generated for each encryption and prepended to the output.

#### Scenario: Encrypt and decrypt round-trip
- **WHEN** a plaintext "hello" is encrypted with a 32-byte key
- **THEN** the resulting ciphertext SHALL be decryptable back to "hello" using the same key

#### Scenario: Different nonces for same plaintext
- **WHEN** the same plaintext is encrypted twice with the same key
- **THEN** the two ciphertexts SHALL differ (due to random nonces)

### Requirement: AES-256-GCM decryption
The system SHALL provide a `Decryptor` class with a `decrypt(ciphertext, key)` method. It SHALL extract the nonce, verify the authentication tag, and return the plaintext. If the tag is invalid, it SHALL throw a `CryptoError`.

#### Scenario: Decrypt with correct key
- **WHEN** a ciphertext is decrypted with the same key used for encryption
- **THEN** the original plaintext SHALL be returned

#### Scenario: Decrypt with wrong key
- **WHEN** a ciphertext is decrypted with a different key
- **THEN** a `CryptoError` SHALL be thrown with an authentication failure message

#### Scenario: Decrypt tampered ciphertext
- **WHEN** a ciphertext byte is modified and then decrypted
- **THEN** a `CryptoError` SHALL be thrown

### Requirement: Key generation
The system SHALL provide `CryptoUtils::generateKey()` that returns a cryptographically random 32-byte key.

#### Scenario: Generate a key
- **WHEN** `generateKey()` is called
- **THEN** the returned key SHALL be 32 bytes and SHALL not be all zeros

#### Scenario: Two keys are different
- **WHEN** `generateKey()` is called twice
- **THEN** the two keys SHALL be different

### Requirement: Secure storage
The system SHALL provide a `SecureStorage` class that stores key-value pairs encrypted on disk. The storage file SHALL be located in the application data directory. Values SHALL be encrypted with AES-256-GCM before writing.

#### Scenario: Store and retrieve a secret
- **WHEN** `SecureStorage::set("api_token", "abc123")` is called and then `SecureStorage::get("api_token")` is called
- **THEN** the returned value SHALL be "abc123"

#### Scenario: Retrieve a non-existent key
- **WHEN** `SecureStorage::get("nonexistent")` is called
- **THEN** an empty QString SHALL be returned

#### Scenario: Overwrite a stored value
- **WHEN** `SecureStorage::set("key", "old")` is called and then `SecureStorage::set("key", "new")` is called
- **THEN** `SecureStorage::get("key")` SHALL return "new"

#### Scenario: Delete a stored value
- **WHEN** `SecureStorage::remove("key")` is called after setting a value for "key"
- **THEN** `SecureStorage::get("key")` SHALL return an empty QString

### Requirement: Secure storage file encryption at rest
The `SecureStorage` file on disk SHALL NOT contain plaintext values. All values SHALL be encrypted. The encryption key itself SHALL be stored using OS-provided secure storage (Keychain, DPAPI, or Secret Service) or derived from a machine-specific value.

#### Scenario: Inspect storage file
- **WHEN** the `SecureStorage` file is read as raw text
- **THEN** no stored secret values SHALL be visible in plaintext

### Requirement: Hash utility
The system SHALL provide `CryptoUtils::sha256(data)` that returns the SHA-256 hash as a hex string.

#### Scenario: Hash known input
- **WHEN** `sha256("hello")` is called
- **THEN** the result SHALL be "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824"

#### Scenario: Hash empty input
- **WHEN** `sha256("")` is called
- **THEN** the result SHALL be the known SHA-256 of an empty string
