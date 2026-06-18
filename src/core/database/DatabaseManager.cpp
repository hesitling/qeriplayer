/// @file DatabaseManager.cpp
/// @brief SQLite database wrapper implementation

#include "core/database/DatabaseManager.h"

#include <sqlite3.h>

#include <QDebug>
#include <algorithm>

namespace QeriPlayerQt {

DatabaseManager::DatabaseManager() = default;

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open(const QString &path)
{
    if (m_db) {
        close();
    }

    int rc = sqlite3_open(path.toUtf8().constData(), &m_db);
    if (rc != SQLITE_OK) {
        qWarning() << "DatabaseManager: failed to open" << path << ":" << sqlite3_errmsg(m_db);
        m_db = nullptr;
        return false;
    }

    // Enable WAL mode for better concurrent read performance
    if (sqlite3_exec(m_db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        qWarning() << "DatabaseManager: PRAGMA journal_mode=WAL failed:" << sqlite3_errmsg(m_db);
    }
    if (sqlite3_exec(m_db, "PRAGMA foreign_keys=ON;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        qWarning() << "DatabaseManager: PRAGMA foreign_keys=ON failed:" << sqlite3_errmsg(m_db);
    }

    ensureSchemaVersionTable();
    runMigrations();

    return true;
}

void DatabaseManager::close()
{
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool DatabaseManager::isOpen() const
{
    return m_db != nullptr;
}

int DatabaseManager::affectedRows() const
{
    return m_affectedRows;
}

int DatabaseManager::schemaVersion() const
{
    return m_currentVersion;
}

void DatabaseManager::registerMigration(int version, std::function<bool(sqlite3 *)> fn)
{
    m_migrations.emplace_back(version, std::move(fn));
}

static void bindVariant(sqlite3_stmt *stmt, int idx, const QVariant &value)
{
    int rc = SQLITE_OK;
    switch (value.typeId()) {
        case QMetaType::QString: {
            QByteArray utf8 = value.toString().toUtf8();
            rc = sqlite3_bind_text(stmt, idx, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
            break;
        }
        case QMetaType::Int:
        case QMetaType::UInt:
            rc = sqlite3_bind_int(stmt, idx, value.toInt());
            break;
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
            rc = sqlite3_bind_int64(stmt, idx, value.toLongLong());
            break;
        case QMetaType::Double:
            rc = sqlite3_bind_double(stmt, idx, value.toDouble());
            break;
        case QMetaType::QByteArray: {
            QByteArray ba = value.toByteArray();
            rc = sqlite3_bind_blob(stmt, idx, ba.constData(), ba.size(), SQLITE_TRANSIENT);
            break;
        }
        case QMetaType::Nullptr:
        case QMetaType::UnknownType:
            rc = sqlite3_bind_null(stmt, idx);
            break;
        default: {
            QByteArray utf8 = value.toString().toUtf8();
            rc = sqlite3_bind_text(stmt, idx, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
            break;
        }
    }
    if (rc != SQLITE_OK) {
        throw DatabaseError("Bind failed at index " + std::to_string(idx) + ": " + sqlite3_errstr(rc));
    }
}

static QVariant readColumn(sqlite3_stmt *stmt, int col)
{
    switch (sqlite3_column_type(stmt, col)) {
        case SQLITE_INTEGER:
            return QVariant(sqlite3_column_int64(stmt, col));
        case SQLITE_FLOAT:
            return QVariant(sqlite3_column_double(stmt, col));
        case SQLITE_TEXT:
            return QVariant(QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(stmt, col)),
                                              sqlite3_column_bytes(stmt, col)));
        case SQLITE_BLOB:
            return QVariant(
                QByteArray(static_cast<const char *>(sqlite3_column_blob(stmt, col)), sqlite3_column_bytes(stmt, col)));
        case SQLITE_NULL:
        default:
            return QVariant();
    }
}

QVector<QueryRow> DatabaseManager::exec(const QString &sql, const QVariantList &params)
{
    if (!m_db) {
        throw DatabaseError("Database not open");
    }

    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string err = sqlite3_errmsg(m_db);
        throw DatabaseError("Prepare failed: " + err);
    }

    // Bind positional parameters
    for (int i = 0; i < params.size(); ++i) {
        bindVariant(stmt, i + 1, params[i]);
    }

    m_affectedRows = 0;

    // Execute and collect results
    QVector<QueryRow> rows;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int colCount = sqlite3_column_count(stmt);
        QueryRow row;
        row.reserve(colCount);
        for (int col = 0; col < colCount; ++col) {
            row.append(readColumn(stmt, col));
        }
        rows.append(std::move(row));
    }

    if (rc != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(m_db);
        sqlite3_finalize(stmt);
        throw DatabaseError("Exec failed: " + err);
    }

    m_affectedRows = sqlite3_changes(m_db);
    sqlite3_finalize(stmt);
    return rows;
}

QVector<QueryRow> DatabaseManager::execNamed(const QString &sql, const QVariantMap &params)
{
    if (!m_db) {
        throw DatabaseError("Database not open");
    }

    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string err = sqlite3_errmsg(m_db);
        throw DatabaseError("Prepare failed: " + err);
    }

    // Bind named parameters
    for (auto it = params.begin(); it != params.end(); ++it) {
        int idx = sqlite3_bind_parameter_index(stmt, it.key().toUtf8().constData());
        if (idx > 0) {
            bindVariant(stmt, idx, it.value());
        }
    }

    m_affectedRows = 0;

    // Execute and collect results
    QVector<QueryRow> rows;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int colCount = sqlite3_column_count(stmt);
        QueryRow row;
        row.reserve(colCount);
        for (int col = 0; col < colCount; ++col) {
            row.append(readColumn(stmt, col));
        }
        rows.append(std::move(row));
    }

    if (rc != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(m_db);
        sqlite3_finalize(stmt);
        throw DatabaseError("Exec failed: " + err);
    }

    m_affectedRows = sqlite3_changes(m_db);
    sqlite3_finalize(stmt);
    return rows;
}

void DatabaseManager::beginTransaction()
{
    if (!m_db) {
        throw DatabaseError(std::string("Database not open"));
    }
    int rc = sqlite3_exec(m_db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        throw DatabaseError(std::string("Begin failed: ") + sqlite3_errmsg(m_db));
    }
}

void DatabaseManager::commitTransaction()
{
    if (!m_db) {
        throw DatabaseError(std::string("Database not open"));
    }
    int rc = sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        throw DatabaseError(std::string("Commit failed: ") + sqlite3_errmsg(m_db));
    }
}

void DatabaseManager::rollbackTransaction()
{
    if (!m_db) {
        throw DatabaseError(std::string("Database not open"));
    }
    int rc = sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        throw DatabaseError(std::string("Rollback failed: ") + sqlite3_errmsg(m_db));
    }
}

void DatabaseManager::ensureSchemaVersionTable()
{
    char *errMsg = nullptr;
    int rc = sqlite3_exec(m_db,
                          "CREATE TABLE IF NOT EXISTS schema_version ("
                          "  version INTEGER NOT NULL PRIMARY KEY"
                          ");",
                          nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "unknown error";
        sqlite3_free(errMsg);
        throw DatabaseError("Failed to create schema_version table: " + err);
    }

    auto rows = exec("SELECT version FROM schema_version");
    if (rows.isEmpty()) {
        exec("INSERT INTO schema_version (version) VALUES (?)", {QVariant(0)});
        m_currentVersion = 0;
    } else {
        m_currentVersion = rows[0][0].toInt();
    }
}

void DatabaseManager::runMigrations()
{
    // Apply initial schema (version 2) if needed
    if (m_currentVersion < 2) {
        beginTransaction();
        try {
            applyInitialSchema(m_db);
            exec("UPDATE schema_version SET version = ?", {QVariant(2)});
            commitTransaction();
            m_currentVersion = 2;
        } catch (...) {
            try {
                rollbackTransaction();
            } catch (const std::exception &rbEx) {
                qWarning() << "DatabaseManager: initial schema rollback failed:" << rbEx.what();
            }
            throw;
        }
    }

    // Apply registered migrations in version order
    std::sort(m_migrations.begin(), m_migrations.end(), [](const auto &a, const auto &b) { return a.first < b.first; });

    for (const auto &[version, fn] : m_migrations) {
        if (m_currentVersion < version) {
            beginTransaction();
            try {
                if (!fn(m_db)) {
                    throw DatabaseError("Migration to version " + std::to_string(version) + " failed");
                }
                exec("UPDATE schema_version SET version = ?", {QVariant(version)});
                commitTransaction();
                m_currentVersion = version;
            } catch (...) {
                try {
                    rollbackTransaction();
                } catch (const std::exception &rbEx) {
                    qWarning() << "DatabaseManager: migration rollback failed:" << rbEx.what();
                }
                throw;
            }
        }
    }
}

void DatabaseManager::applyInitialSchema(sqlite3 *handle)
{
    const char *sql = R"SQL(
        CREATE TABLE IF NOT EXISTS songs_cache (
            id                   TEXT PRIMARY KEY,
            platform             TEXT,
            name                 TEXT,
            artist               TEXT,
            album                TEXT,
            album_id             TEXT,
            duration_ms          INTEGER,
            cover_url            TEXT,
            media_uri            TEXT,
            custom_name          TEXT,
            custom_artist        TEXT,
            custom_cover_url     TEXT,
            original_name        TEXT,
            original_artist      TEXT,
            original_cover_url   TEXT,
            local_file_name      TEXT,
            local_file_path      TEXT,
            matched_lyric_source TEXT,
            matched_song_id      TEXT,
            user_lyric_offset_ms INTEGER DEFAULT 0,
            lyrics_json          TEXT,
            channel_id           TEXT,
            audio_id             TEXT,
            sub_audio_id         TEXT,
            extra_json           TEXT,
            cached_at            TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            last_played_at       TIMESTAMP
        );

        CREATE TABLE IF NOT EXISTS playlists (
            id               TEXT PRIMARY KEY,
            platform         TEXT,
            name             TEXT,
            description      TEXT,
            cover_url        TEXT,
            song_count       INTEGER DEFAULT 0,
            owner            TEXT,
            custom_cover_url TEXT,
            modified_at      INTEGER DEFAULT 0
        );

        CREATE TABLE IF NOT EXISTS playlist_songs (
            playlist_id TEXT,
            song_id     TEXT,
            position    INTEGER,
            PRIMARY KEY (playlist_id, song_id),
            FOREIGN KEY (playlist_id) REFERENCES playlists(id) ON DELETE CASCADE,
            FOREIGN KEY (song_id) REFERENCES songs_cache(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS settings (
            key   TEXT PRIMARY KEY,
            value TEXT
        );

        CREATE TABLE IF NOT EXISTS play_history (
            id       INTEGER PRIMARY KEY AUTOINCREMENT,
            song_id  TEXT,
            played_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );

        CREATE TABLE IF NOT EXISTS player_state (
            id             INTEGER PRIMARY KEY CHECK(id = 1),
            playlist_json  TEXT,
            current_index  INTEGER DEFAULT 0,
            media_url      TEXT,
            position_ms    INTEGER DEFAULT 0,
            should_resume  INTEGER DEFAULT 0,
            repeat_mode    INTEGER DEFAULT 0,
            shuffle_enabled INTEGER DEFAULT 0,
            updated_at     TIMESTAMP
        );
    )SQL";

    char *errMsg = nullptr;
    int rc = sqlite3_exec(handle, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "unknown error";
        sqlite3_free(errMsg);
        throw DatabaseError("Initial schema failed: " + err);
    }
}

} // namespace QeriPlayerQt
