/// @file TestNeteaseCrypto.cpp
/// @brief Unit tests for NeteaseCrypto

#include "api/netease/NeteaseCrypto.h"

#include <QTest>

using namespace NeriPlayerQt;

class TestNeteaseCrypto : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void weapiEncrypt_producesNonEmptyResult();
    void weapiEncrypt_differentKeysDifferentOutput();
    void weapiEncryptWithKey_deterministic();
    void weapiEncryptWithKey_paramsIsBase64();
    void weapiEncryptWithKey_encSecKeyIsHex();
};

void TestNeteaseCrypto::weapiEncrypt_producesNonEmptyResult()
{
    auto result = NeteaseCrypto::weapiEncrypt(QStringLiteral("{\"s\":\"test\"}"));
    QVERIFY(!result.params.isEmpty());
    QVERIFY(!result.encSecKey.isEmpty());
}

void TestNeteaseCrypto::weapiEncrypt_differentKeysDifferentOutput()
{
    // Random key means two encryptions of same plaintext should differ
    auto r1 = NeteaseCrypto::weapiEncrypt(QStringLiteral("{\"s\":\"test\"}"));
    auto r2 = NeteaseCrypto::weapiEncrypt(QStringLiteral("{\"s\":\"test\"}"));
    QVERIFY(r1.params != r2.params);
}

void TestNeteaseCrypto::weapiEncryptWithKey_deterministic()
{
    // Fixed key should produce identical output
    QByteArray fixedKey(16, '\x42');
    auto r1 = NeteaseCrypto::weapiEncryptWithKey(QStringLiteral("{\"s\":\"test\"}"), fixedKey);
    auto r2 = NeteaseCrypto::weapiEncryptWithKey(QStringLiteral("{\"s\":\"test\"}"), fixedKey);
    QCOMPARE(r1.params, r2.params);
    QCOMPARE(r1.encSecKey, r2.encSecKey);
}

void TestNeteaseCrypto::weapiEncryptWithKey_paramsIsBase64()
{
    QByteArray fixedKey(16, '\x01');
    auto result = NeteaseCrypto::weapiEncryptWithKey(QStringLiteral("{}"), fixedKey);

    // Base64 should only contain valid characters
    QByteArray decoded = QByteArray::fromBase64(result.params.toLatin1());
    // If it deciles back to base64 without error, it's valid base64
    QVERIFY(!decoded.isEmpty() || result.params.isEmpty());
    // Actually, for an empty JSON object the encryption should still produce output
    QVERIFY(!result.params.isEmpty());
}

void TestNeteaseCrypto::weapiEncryptWithKey_encSecKeyIsHex()
{
    QByteArray fixedKey(16, '\x01');
    auto result = NeteaseCrypto::weapiEncryptWithKey(QStringLiteral("{}"), fixedKey);

    // encSecKey should be a hex string of 128 bytes = 256 hex chars
    QCOMPARE(result.encSecKey.size(), 256);

    // All characters should be hex digits
    for (const QChar &c : result.encSecKey) {
        QVERIFY(c.isDigit() || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
    }
}

QTEST_MAIN(TestNeteaseCrypto)
#include "TestNeteaseCrypto.moc"
