# AGENTS.md

Guidelines for AI agents working on NeriPlayer Qt.

## Tech Stack

| Category | Technology | Version |
|----------|-----------|---------|
| Language | C++ | 20 |
| Coroutine | QCoro | 0.10+ |
| UI Framework | Qt | 6.5+ |
| Build System | CMake | 3.16+ |
| Database | SQLite | 3.35+ |
| JSON | nlohmann/json | 3.11+ |
| Logging | spdlog | 1.12+ |

## Project Structure

```
src/
├── app/                    # Application entry
├── core/                   # Infrastructure (network, database, crypto, logger)
├── api/                    # Platform APIs (netease, bilibili, youtube)
├── player/                 # Audio playback
├── playlist/               # Playlist management
├── download/               # Download manager
├── search/                 # Search aggregation
├── sync/                   # Cloud sync (GitHub/WebDAV)
├── settings/               # Settings management
├── ui/                     # UI components (widgets, dialogs, views)
└── viewmodel/              # ViewModels (MVVM)
```

## Code Conventions

### Naming

| Type | Convention | Example |
|------|-----------|---------|
| Files | `PascalCase.h/cpp` | `HttpClient.h` |
| Classes | `PascalCase` | `PlayerManager` |
| Interfaces | `IPascalCase` | `IMusicPlatformPlugin` |
| Member vars | `m_camelCase` | `m_networkManager` |
| Local vars | `camelCase` | `statusCode` |
| Constants | `UPPER_SNAKE_CASE` | `MAX_RETRY_COUNT` |
| Functions | `camelCase` | `getSongDetail()` |
| Signals | `camelCase` | `stateChanged()` |

### Coroutine Style

```cpp
QCoro::Task<SearchResult> search(const QString &query) {
    auto *reply = co_await m_manager->get(request);
    if (reply->error() != QNetworkReply::NoError) {
        throw NetworkError(reply->errorString());
    }
    co_return parseResult(reply->readAll());
}
```

### Include Order

1. Corresponding header
2. QCoro headers
3. Qt headers
4. Standard library
5. Third-party libraries

### Exception Handling

- **Never swallow exceptions silently.** Every `catch` block must either log a warning, re-throw, or both.
- Use `qWarning()` in low-level modules (`core/`) that should not depend on Logger.
- Use `Logger::get(category)->warn()` in higher layers.
- Rollback failures in nested try/catch should log and swallow (the original exception takes precedence):
  ```cpp
  try {
      // ...
  } catch (...) {
      try { rollbackTransaction(); } catch (const std::exception &ex) {
          qWarning() << "rollback failed:" << ex.what();
      }
      throw;
  }
  ```
- Prefer RAII and `std::unique_ptr` over manual try/catch for resource cleanup.

### Qt Types

- Use `QString` for all user-facing text and internal strings (not `std::string`).
- `QUrl` handles all URLs (not `QString`).
- Durations (milliseconds), timestamps (epoch ms), and file sizes go in `qint64`.
- All domain types passed through QVariant must use `Q_DECLARE_METATYPE`.
- Domain structs are plain value types — no `QObject`, no `Q_GADGET`.

### C++ Includes

- Every header must explicitly include what it uses — never rely on transitive includes for standard types (`<stdexcept>`, `<cstdint>`, `<QString>`, etc.).

### SQLite

- Always wrap multi-statement write operations in transactions (`beginTransaction`/`commitTransaction`).
- Check return codes on all `sqlite3_exec` calls — never pass `nullptr` for the error output.
- Use parameterized queries (`?` or `:param`) — never concatenate user input into SQL.

### QCoro

- All async functions must return `QCoro::Task<T>` and use `co_await`/`co_return`.
- Do not mix `QFuture` and `QCoro::Task` without explicit conversion.

### spdlog

- Use `daily_file_sink_mt` for log file rotation (not `rotating_file_sink_mt`).
- Cache logger instances — do not call `Logger::get()` in hot paths.

### Memory & Safety

- Own heap objects with `std::unique_ptr` — no raw `new`/`delete`.
- Use `thread_local` for per-thread mutable state (not `static`).
- Set restrictive file permissions (0600) on sensitive files (keys, tokens).

## Length Guidelines

| Type | Max | Recommended |
|------|-----|-------------|
| Header file | 200 lines | 100-150 |
| Source file | 500 lines | 200-400 |
| Function | 50 lines | 20-30 |
| Coroutine | 80 lines | 30-50 |
| Line width | 120 chars | 80-100 |

## Commit Conventions

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <subject>
```

### Types

| Type | Usage |
|------|-------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation |
| `refactor` | Code refactoring |
| `test` | Add/update tests |
| `chore` | Other changes |

### Scopes

`app`, `core`, `api`, `player`, `ui`, `docs`, `build`, `test`

### Examples

```
feat(api): add YouTube Music search support
fix(network): handle connection timeout
refactor(core): use QCoro for async operations
docs: update architecture documentation
```

### Rules

- Use imperative mood ("add" not "added")
- No capitalization on first letter
- No period at end
- Max 50 characters for subject
- Reference issues: `Fixes #123`

## Doxygen Documentation

Use `///` for brief comments, `/** */` for detailed documentation. Key conventions:

- **File header:** `/// @file`, `/// @brief`, `/// @date`
- **Classes:** `@brief`, `@code` example, `@see` cross-references
- **Functions:** `@brief`, `@param`, `@return`, `@throws`
- **Enums/Members:** `///< inline brief` after each value
- **Properties:** `@brief`, `@see` the backing enum/type

## Testing

```cpp
class HttpClientTest : public QObject {
    Q_OBJECT
private slots:
    void testGet_ValidUrl_ReturnsSuccess();
    void testGet_Timeout_ReturnsError();
};

void HttpClientTest::testGet_ValidUrl_ReturnsSuccess() {
    QCoro::waitFor([]() -> QCoro::Task<void> {
        HttpClient client;
        auto response = co_await client.get(QUrl("https://example.com"));
        QVERIFY(response.isSuccess());
    }());
}
```

## References

- [Qt 6 Docs](https://doc.qt.io/qt-6/)
- [QCoro Docs](https://qcoro.dev/)
- [Doxygen Manual](https://www.doxygen.nl/manual/)
- [Conventional Commits](https://www.conventionalcommits.org/)
