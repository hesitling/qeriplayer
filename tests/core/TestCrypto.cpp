/// @file TestCrypto.cpp
/// @brief Unit tests for the crypto module
/// @date 2024-01-15

#include "core/crypto/CryptoUtils.h"
#include "core/crypto/Decryptor.h"
#include "core/crypto/Encryptor.h"
#include "core/crypto/SecureStorage.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

using namespace NeriPlayerQt;

class TestCrypto : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Encryptor / Decryptor
    void encrypt_decrypt_roundTrip();
    void encrypt_differentNonces();
    void decrypt_wrongKey_throws();
    void decrypt_tamperedCiphertext_throws();

    // CryptoUtils
    void generateKey_returns32Bytes();
    void generateKey_differentEachTime();
    void sha256_knownInput();
    void sha256_emptyInput();

    // SecureStorage
    void secureStorage_setAndGet();
    void secureStorage_getNonExistent();
    void secureStorage_overwrite();
    void secureStorage_remove();
    void secureStorage_encryptedAtRest();
    void secureStorage_malformedFileGraceful();
    void secureStorage_corruptedSingleValueIsIsolated();
};

void TestCrypto::encrypt_decrypt_roundTrip()
{
    QByteArray key = CryptoUtils::generateKey();
    QCOMPARE(key.size(), 32);

    QByteArray plaintext = "hello world";
    QByteArray ciphertext = Encryptor::encrypt(plaintext, key);

    QVERIFY(!ciphertext.isEmpty());
    QVERIFY(ciphertext != plaintext);

    QByteArray decrypted = Decryptor::decrypt(ciphertext, key);
    QCOMPARE(decrypted, plaintext);
}

void TestCrypto::encrypt_differentNonces()
{
    QByteArray key = CryptoUtils::generateKey();
    QByteArray plaintext = "same content";

    QByteArray ct1 = Encryptor::encrypt(plaintext, key);
    QByteArray ct2 = Encryptor::encrypt(plaintext, key);

    // Should be different due to random nonces
    QVERIFY(ct1 != ct2);
}

void TestCrypto::decrypt_wrongKey_throws()
{
    QByteArray key1 = CryptoUtils::generateKey();
    QByteArray key2 = CryptoUtils::generateKey();

    QByteArray ciphertext = Encryptor::encrypt("secret", key1);

    QVERIFY_EXCEPTION_THROWN(Decryptor::decrypt(ciphertext, key2), CryptoError);
}

void TestCrypto::decrypt_tamperedCiphertext_throws()
{
    QByteArray key = CryptoUtils::generateKey();
    QByteArray ciphertext = Encryptor::encrypt("data", key);

    // Tamper with a byte
    QByteArray tampered = ciphertext;
    if (!tampered.isEmpty()) {
        tampered[tampered.size() / 2] ^= 0xFF;
    }

    QVERIFY_EXCEPTION_THROWN(Decryptor::decrypt(tampered, key), CryptoError);
}

void TestCrypto::generateKey_returns32Bytes()
{
    QByteArray key = CryptoUtils::generateKey();
    QCOMPARE(key.size(), 32);

    // Should not be all zeros
    bool hasNonZero = false;
    for (char c : key) {
        if (c != 0) {
            hasNonZero = true;
            break;
        }
    }
    QVERIFY(hasNonZero);
}

void TestCrypto::generateKey_differentEachTime()
{
    QByteArray key1 = CryptoUtils::generateKey();
    QByteArray key2 = CryptoUtils::generateKey();
    QVERIFY(key1 != key2);
}

void TestCrypto::sha256_knownInput()
{
    // SHA-256 of "hello"
    QByteArray hash = CryptoUtils::sha256("hello");
    QCOMPARE(hash, QByteArray("2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824"));
}

void TestCrypto::sha256_emptyInput()
{
    QByteArray hash = CryptoUtils::sha256("");
    QCOMPARE(hash, QByteArray("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
}

void TestCrypto::secureStorage_setAndGet()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SecureStorage storage(tempDir.filePath("secrets.dat"));
    storage.set("api_token", "abc123");
    QCOMPARE(storage.get("api_token"), QString("abc123"));
}

void TestCrypto::secureStorage_getNonExistent()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SecureStorage storage(tempDir.filePath("secrets.dat"));
    QCOMPARE(storage.get("nonexistent"), QString());
}

void TestCrypto::secureStorage_overwrite()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SecureStorage storage(tempDir.filePath("secrets.dat"));
    storage.set("key", "old");
    storage.set("key", "new");
    QCOMPARE(storage.get("key"), QString("new"));
}

void TestCrypto::secureStorage_remove()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SecureStorage storage(tempDir.filePath("secrets.dat"));
    storage.set("key", "value");
    storage.remove("key");
    QCOMPARE(storage.get("key"), QString());
}

void TestCrypto::secureStorage_encryptedAtRest()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.filePath("secrets.dat");
    SecureStorage storage(filePath);
    storage.set("my_secret", "super_secret_value");

    // Read raw file content — should not contain plaintext
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray rawContent = file.readAll();
    QVERIFY(!rawContent.contains("super_secret_value"));
}

void TestCrypto::secureStorage_malformedFileGraceful()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.filePath("secrets.dat");

    // Write non-JSON content
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write("this is not json");
    file.close();

    // Should not crash, should return empty for any key
    SecureStorage storage(filePath);
    QCOMPARE(storage.get("any_key"), QString());
}

void TestCrypto::secureStorage_corruptedSingleValueIsIsolated()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.filePath("secrets.dat");

    // Create valid storage with two keys
    {
        SecureStorage storage(filePath);
        storage.set("good_key", "good_value");
        storage.set("bad_key", "bad_value");
    }

    // Corrupt only bad_key's value
    {
        QFile file(filePath);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QByteArray raw = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(raw);
        QVERIFY(doc.isObject());
        QJsonObject obj = doc.object();
        QVERIFY(obj.contains("bad_key"));

        // Overwrite with invalid ciphertext
        obj["bad_key"] = QStringLiteral("%%%invalid_ciphertext%%%");
        doc.setObject(obj);

        QFile writeFile(filePath);
        QVERIFY(writeFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
        writeFile.write(doc.toJson(QJsonDocument::Compact));
        writeFile.close();
    }

    // Re-open: good_key should work, bad_key should return empty
    SecureStorage storage(filePath);
    QCOMPARE(storage.get("good_key"), QString("good_value"));
    QCOMPARE(storage.get("bad_key"), QString());
}

QTEST_MAIN(TestCrypto)
#include "TestCrypto.moc"
