# Database Module (database/)

## 1. Overview

The database module provides SQLite database operation support, including table management, data migration, query building, and transaction handling.

## 2. Directory Structure

```
src/core/database/
├── DatabaseManager.h/.cpp    # Database manager
├── QueryBuilder.h/.cpp       # Query builder
└── MigrationManager.h/.cpp   # Migration manager
```

## 3. Main Class Design

### 3.1 DatabaseManager

Database manager that manages database connections and operations.

```cpp
class DatabaseManager : public QObject {
    Q_OBJECT
public:
    explicit DatabaseManager(const QString &dbPath, 
                             QObject *parent = nullptr);
    ~DatabaseManager();
    
    // Initialize database
    bool initialize();
    
    // Execute query
    QSqlQuery execute(const QString &sql, 
                      const QVariantMap &params = {});
    
    // Transaction management
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    
    // Table operations
    bool createTable(const QString &tableName, 
                     const TableSchema &schema);
    bool dropTable(const QString &tableName);
    bool tableExists(const QString &tableName) const;
    bool alterTable(const QString &tableName,
                    const TableSchema &newSchema);
    
    // Data operations
    bool insert(const QString &table, const QVariantMap &data);
    bool update(const QString &table, const QVariantMap &data,
                const QString &where, const QVariantMap &params = {});
    bool remove(const QString &table, 
                const QString &where, const QVariantMap &params = {});
    
    // Query
    QSqlQuery select(const QString &table, 
                     const QStringList &columns = {},
                     const QString &where = {},
                     const QVariantMap &params = {},
                     const QString &orderBy = {},
                     int limit = -1, int offset = -1);
    
    // Get last inserted ID
    qint64 lastInsertId() const;
    
    // Get affected row count
    int affectedRows() const;
    
    // Database path
    QString databasePath() const;
    
    // Database size
    qint64 databaseSize() const;
    
    // Compact database
    bool vacuum();
    
signals:
    void databaseOpened();
    void databaseClosed();
    void databaseError(const QString &error);
    
private:
    bool openDatabase();
    void closeDatabase();
    bool runMigrations();
    
    QString m_dbPath;
    QSqlDatabase m_database;
    std::unique_ptr<MigrationManager> m_migrationManager;
};
```

### 3.2 QueryBuilder

Query builder that provides chain-style query construction.

```cpp
class QueryBuilder {
public:
    QueryBuilder(const QString &table);
    
    // SELECT
    QueryBuilder& select(const QStringList &columns);
    QueryBuilder& selectAll();
    QueryBuilder& selectDistinct(const QStringList &columns);
    
    // WHERE
    QueryBuilder& where(const QString &condition, const QVariant &value);
    QueryBuilder& where(const QString &condition);
    QueryBuilder& whereIn(const QString &column, const QVariantList &values);
    QueryBuilder& whereNotIn(const QString &column, const QVariantList &values);
    QueryBuilder& whereNull(const QString &column);
    QueryBuilder& whereNotNull(const QString &column);
    QueryBuilder& whereBetween(const QString &column, 
                               const QVariant &from, const QVariant &to);
    QueryBuilder& whereLike(const QString &column, const QString &pattern);
    
    // JOIN
    QueryBuilder& join(const QString &table, const QString &condition,
                       JoinType type = JoinType::Inner);
    QueryBuilder& leftJoin(const QString &table, const QString &condition);
    QueryBuilder& rightJoin(const QString &table, const QString &condition);
    
    // ORDER BY
    QueryBuilder& orderBy(const QString &column, 
                          Qt::SortOrder order = Qt::AscendingOrder);
    QueryBuilder& orderByDesc(const QString &column);
    
    // GROUP BY
    QueryBuilder& groupBy(const QStringList &columns);
    QueryBuilder& having(const QString &condition, const QVariant &value);
    
    // LIMIT / OFFSET
    QueryBuilder& limit(int limit);
    QueryBuilder& offset(int offset);
    
    // Build query
    QString toSql() const;
    QVariantMap bindings() const;
    
    // Execute query
    QSqlQuery execute(DatabaseManager *db) const;
    
private:
    QString m_table;
    QStringList m_columns;
    QStringList m_whereConditions;
    QVariantMap m_bindings;
    QStringList m_joins;
    QStringList m_orderBy;
    QStringList m_groupBy;
    QString m_having;
    int m_limit = -1;
    int m_offset = 0;
    bool m_distinct = false;
};

// JOIN type
enum class JoinType {
    Inner,
    Left,
    Right,
    Cross
};
```

### 3.3 MigrationManager

Migration manager that manages database versions and migrations.

```cpp
class MigrationManager {
public:
    explicit MigrationManager(DatabaseManager *db);
    
    // Register migration
    void addMigration(int version, 
                      std::function<bool(DatabaseManager*)> migrate);
    
    // Execute migration
    bool migrate();
    
    // Get current version
    int currentVersion() const;
    
    // Get target version
    int targetVersion() const;
    
    // Check if migration is needed
    bool needsMigration() const;
    
private:
    bool createMigrationTable();
    bool runMigration(int version);
    
    DatabaseManager *m_db;
    QMap<int, std::function<bool(DatabaseManager*)>> m_migrations;
    int m_currentVersion = 0;
};
```

### 3.4 TableSchema

Table schema definition.

```cpp
struct TableSchema {
    struct Column {
        QString name;
        QString type;
        bool primaryKey = false;
        bool autoIncrement = false;
        bool notNull = false;
        QVariant defaultValue;
        QString references;  // Foreign key reference
    };
    
    QList<Column> columns;
    QStringList indices;
    QStringList uniqueConstraints;
};
```

## 4. Usage Examples

### 4.1 Initialize Database

```cpp
auto *db = new DatabaseManager("app.db");
if (!db->initialize()) {
    LOG_ERROR("Database", "Failed to initialize database");
    return;
}
```

### 4.2 Create Table

```cpp
TableSchema schema;
schema.columns = {
    {"id", "INTEGER", true, true},
    {"title", "TEXT", false, false, true},
    {"artist", "TEXT"},
    {"duration", "INTEGER"},
    {"created_at", "DATETIME", false, false, false, "CURRENT_TIMESTAMP"}
};
schema.indices = {"title", "artist"};

db->createTable("songs", schema);
```

### 4.3 Insert Data

```cpp
QVariantMap data;
data["title"] = "Song Title";
data["artist"] = "Artist Name";
data["duration"] = 180000;
db->insert("songs", data);
```

### 4.4 Query Builder

```cpp
// Complex query
auto query = QueryBuilder("songs")
    .select({"songs.*", "playlists.name as playlist_name"})
    .join("playlist_songs", "songs.id = playlist_songs.song_id")
    .join("playlists", "playlist_songs.playlist_id = playlists.id")
    .where("songs.artist", "Artist Name")
    .orderBy("songs.title")
    .limit(50)
    .offset(0);

auto result = query.execute(db);
while (result.next()) {
    // Process results
}
```

### 4.5 Transaction Handling

```cpp
db->beginTransaction();
try {
    db->insert("songs", songData1);
    db->insert("songs", songData2);
    db->commitTransaction();
} catch (...) {
    db->rollbackTransaction();
    throw;
}
```

## 5. Testing

```cpp
class DatabaseManagerTest : public QObject {
    Q_OBJECT
private slots:
    void testCreateTable();
    void testInsert();
    void testSelect();
    void testUpdate();
    void testDelete();
    void testTransaction();
    void testMigration();
};
```

## 6. Summary

The database module provides complete SQLite database support:
- **DatabaseManager**: Database connection, table management, CRUD operations
- **QueryBuilder**: Chain-style query construction, parameter binding
- **MigrationManager**: Database version management, migration execution
