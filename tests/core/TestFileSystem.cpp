/// @file TestFileSystem.cpp
/// @brief Unit tests for the filesystem module

#include "core/filesystem/AppPaths.h"
#include "core/filesystem/FileUtils.h"
#include "core/filesystem/FileWatcher.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

using namespace QeriPlayerQt;

class TestFileSystem : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // AppPaths
    void dataDir_returnsValidPath();
    void appPaths_useStandardLocations();
    void appPaths_useIsolatedLocations();
    void dataDir_autoCreates();
    void configDir_returnsValidPath();
    void configDir_autoCreates();
    void cacheDir_returnsValidPath();
    void cacheDir_autoCreates();
    void tempDir_returnsValidPath();
    void tempDir_usesIsolatedTemp();
    void tempDir_autoCreates();

    // FileUtils
    void ensureDir_createsNestedDirs();
    void ensureDir_alreadyExists();
    void readFile_existingFile();
    void readFile_nonExistent();
    void writeFile_createsFile();
    void writeFile_overwrite();

    // FileWatcher
    void fileWatcher_emitsSignalOnChange();
    void fileWatcher_stop();
};

void TestFileSystem::dataDir_returnsValidPath()
{
    QString path = AppPaths::dataDir();
    QVERIFY(!path.isEmpty());
    QVERIFY(QDir(path).exists());
}

void TestFileSystem::appPaths_useStandardLocations()
{
    QCOMPARE(AppPaths::dataDir(),
             QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/QeriPlayer"));
    QCOMPARE(AppPaths::configDir(),
             QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/QeriPlayer"));
    QCOMPARE(AppPaths::cacheDir(),
             QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QStringLiteral("/QeriPlayer"));
}

void TestFileSystem::appPaths_useIsolatedLocations()
{
    const QString testDataHome = QString::fromUtf8(qgetenv("XDG_DATA_HOME"));
    const QString testConfigHome = QString::fromUtf8(qgetenv("XDG_CONFIG_HOME"));
    const QString testCacheHome = QString::fromUtf8(qgetenv("XDG_CACHE_HOME"));

    QVERIFY(!testDataHome.isEmpty());
    QVERIFY(!testConfigHome.isEmpty());
    QVERIFY(!testCacheHome.isEmpty());
    QVERIFY(AppPaths::dataDir().startsWith(testDataHome));
    QVERIFY(AppPaths::configDir().startsWith(testConfigHome));
    QVERIFY(AppPaths::cacheDir().startsWith(testCacheHome));
}

void TestFileSystem::dataDir_autoCreates()
{
    QString path = AppPaths::dataDir();
    QVERIFY(QDir(path).removeRecursively());
    QVERIFY(!QDir(path).exists());
    path = AppPaths::dataDir();
    QVERIFY(!path.isEmpty());
    QVERIFY(QDir(path).exists());
}

void TestFileSystem::configDir_returnsValidPath()
{
    QString path = AppPaths::configDir();
    QVERIFY(!path.isEmpty());
    QVERIFY(QDir(path).exists());
}

void TestFileSystem::configDir_autoCreates()
{
    QString path = AppPaths::configDir();
    QVERIFY(QDir(path).removeRecursively());
    QVERIFY(!QDir(path).exists());
    path = AppPaths::configDir();
    QVERIFY(!path.isEmpty());
    QVERIFY(QDir(path).exists());
}

void TestFileSystem::cacheDir_returnsValidPath()
{
    QString path = AppPaths::cacheDir();
    QVERIFY(!path.isEmpty());
    QVERIFY(QDir(path).exists());
}

void TestFileSystem::cacheDir_autoCreates()
{
    QString path = AppPaths::cacheDir();
    QVERIFY(QDir(path).removeRecursively());
    QVERIFY(!QDir(path).exists());
    path = AppPaths::cacheDir();
    QVERIFY(!path.isEmpty());
    QVERIFY(QDir(path).exists());
}

void TestFileSystem::tempDir_returnsValidPath()
{
    QString path = AppPaths::tempDir();
    QVERIFY(!path.isEmpty());
    QVERIFY(QDir(path).exists());
}

void TestFileSystem::tempDir_usesIsolatedTemp()
{
    const QString testTempDir = QString::fromUtf8(qgetenv("TMPDIR"));
    QVERIFY(!testTempDir.isEmpty());
    QVERIFY(AppPaths::tempDir().startsWith(testTempDir));
}

void TestFileSystem::tempDir_autoCreates()
{
    QString path = AppPaths::tempDir();
    QVERIFY(QDir(path).removeRecursively());
    QVERIFY(!QDir(path).exists());
    path = AppPaths::tempDir();
    QVERIFY(!path.isEmpty());
    QVERIFY(QDir(path).exists());
}

void TestFileSystem::ensureDir_createsNestedDirs()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString nested = tempDir.filePath("a/b/c");
    QVERIFY(!QDir(nested).exists());

    QVERIFY(FileUtils::ensureDir(nested));
    QVERIFY(QDir(nested).exists());
}

void TestFileSystem::ensureDir_alreadyExists()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QVERIFY(FileUtils::ensureDir(tempDir.path()));
    // Should not error
}

void TestFileSystem::readFile_existingFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.filePath("test.txt");
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("hello world");
    file.close();

    QByteArray content = FileUtils::readFile(filePath);
    QCOMPARE(content, QByteArray("hello world"));
}

void TestFileSystem::readFile_nonExistent()
{
    QByteArray content = FileUtils::readFile("/nonexistent/path/file.txt");
    QVERIFY(content.isEmpty());
    QVERIFY(!FileUtils::lastError().isEmpty());
}

void TestFileSystem::writeFile_createsFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.filePath("write_test.txt");
    QVERIFY(FileUtils::writeFile(filePath, "test content"));

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), QByteArray("test content"));
}

void TestFileSystem::writeFile_overwrite()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.filePath("overwrite.txt");

    // Write initial content
    QVERIFY(FileUtils::writeFile(filePath, "original"));

    // Overwrite
    QVERIFY(FileUtils::writeFile(filePath, "updated"));

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), QByteArray("updated"));
}

void TestFileSystem::fileWatcher_emitsSignalOnChange()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.filePath("watched.txt");
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("initial");
    file.close();

    FileWatcher watcher;
    QSignalSpy spy(&watcher, &FileWatcher::fileChanged);
    QVERIFY(spy.isValid());

    watcher.watch(filePath);
    QVERIFY(watcher.isWatching());

    // Modify the file
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Append));
    file.write("more");
    file.close();

    QTRY_VERIFY_WITH_TIMEOUT(spy.count() > 0, 2000);
}

void TestFileSystem::fileWatcher_stop()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.filePath("watched2.txt");
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("data");
    file.close();

    FileWatcher watcher;
    watcher.watch(filePath);
    QVERIFY(watcher.isWatching());

    watcher.stop();
    QVERIFY(!watcher.isWatching());
}

int main(int argc, char **argv)
{
    QTemporaryDir testRoot;
    if (!testRoot.isValid()) {
        return 1;
    }

    const QByteArray rootPath = testRoot.path().toUtf8();
    qputenv("HOME", rootPath);
    qputenv("XDG_DATA_HOME", rootPath + "/data");
    qputenv("XDG_CONFIG_HOME", rootPath + "/config");
    qputenv("XDG_CACHE_HOME", rootPath + "/cache");
    qputenv("TMPDIR", rootPath + "/temp");

    QCoreApplication app(argc, argv);
    TestFileSystem test;
    return QTest::qExec(&test, argc, argv);
}

#include "TestFileSystem.moc"
