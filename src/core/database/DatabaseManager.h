/// @file DatabaseManager.h
/// @brief SQLite database wrapper with schema management

#ifndef QERIPLAYERQT_DATABASEMANAGER_H
#define QERIPLAYERQT_DATABASEMANAGER_H

#include <QString>
#include <QVariant>
#include <QVector>

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

struct sqlite3;

namespace QeriPlayerQt {

/**
 * @brief Exception thrown on database errors
 */
class DatabaseError : public std::runtime_error {
public:
    explicit DatabaseError(const std::string &message)
        : std::runtime_error(message)
    {
    }
};

/**
 * @brief A row of query results as QVariant values
 */
using QueryRow = QVector<QVariant>;

/**
 * @brief Manages SQLite database lifecycle, schema, and queries
 */
class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    /**
     * @brief Open or create a database file
     * @param path Path to the SQLite file
     * @return true on success
     */
    bool open(const QString &path);

    /**
     * @brief Close the database and flush pending writes
     */
    void close();

    /**
     * @brief Check if the database is open
     */
    bool isOpen() const;

    /**
     * @brief Get the number of rows affected by the last INSERT/UPDATE/DELETE
     */
    int affectedRows() const;

    /**
     * @brief Get the current schema version
     */
    int schemaVersion() const;

    /**
     * @brief Register a migration function for a given version
     *
     * Migrations run inside open(). Register all migrations before calling open().
     * Migrations registered after open() will not be applied until the next open().
     *
     * @param version Target version number
     * @param fn Function that receives sqlite3* and returns true on success
     */
    void registerMigration(int version, std::function<bool(sqlite3 *)> fn);

    /**
     * @brief Execute a SQL statement with positional parameters
     * @param sql SQL string with ? placeholders
     * @param params Parameters to bind (1-indexed positional)
     * @return Rows for SELECT queries, empty for others
     * @throws DatabaseError on failure
     */
    QVector<QueryRow> exec(const QString &sql, const QVariantList &params = {});

    /**
     * @brief Execute a SQL statement with named parameters
     * @param sql SQL string with :param placeholders
     * @param params Map of parameter names (with ":") to values
     * @return Rows for SELECT queries, empty for others
     * @throws DatabaseError on failure
     */
    QVector<QueryRow> execNamed(const QString &sql, const QVariantMap &params);

    /**
     * @brief Begin a transaction
     * @throws DatabaseError on failure
     */
    void beginTransaction();

    /**
     * @brief Commit the current transaction
     * @throws DatabaseError on failure
     */
    void commitTransaction();

    /**
     * @brief Rollback the current transaction
     * @throws DatabaseError on failure
     */
    void rollbackTransaction();

private:
    void ensureSchemaVersionTable();
    void runMigrations();
    void applyInitialSchema(sqlite3 *handle);

    sqlite3 *m_db = nullptr;
    int m_currentVersion = 0;
    int m_affectedRows = 0;
    std::vector<std::pair<int, std::function<bool(sqlite3 *)>>> m_migrations;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_DATABASEMANAGER_H
