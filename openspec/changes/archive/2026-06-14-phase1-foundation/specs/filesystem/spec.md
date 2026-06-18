## ADDED Requirements

### Requirement: Application data directory
The system SHALL provide `AppPaths::dataDir()` that returns the platform-appropriate directory for persistent application data:
- Linux: `~/.local/share/QeriPlayer/`
- macOS: `~/Library/Application Support/QeriPlayer/`
- Windows: `%APPDATA%/QeriPlayer/`

The directory SHALL be created if it does not exist.

#### Scenario: Get data directory on Linux
- **WHEN** `AppPaths::dataDir()` is called on Linux
- **THEN** the returned path SHALL end with `.local/share/QeriPlayer`

#### Scenario: Auto-create data directory
- **WHEN** `AppPaths::dataDir()` is called and the directory does not exist
- **THEN** the directory SHALL be created and the path SHALL be returned

### Requirement: Application config directory
The system SHALL provide `AppPaths::configDir()` that returns the platform-appropriate directory for configuration files:
- Linux: `~/.config/QeriPlayer/`
- macOS: `~/Library/Application Support/QeriPlayer/`
- Windows: `%APPDATA%/QeriPlayer/`

The directory SHALL be created if it does not exist.

#### Scenario: Get config directory
- **WHEN** `AppPaths::configDir()` is called
- **THEN** the returned path SHALL be a valid directory that exists

### Requirement: Application cache directory
The system SHALL provide `AppPaths::cacheDir()` that returns the platform-appropriate directory for cache files:
- Linux: `~/.cache/QeriPlayer/`
- macOS: `~/Library/Caches/QeriPlayer/`
- Windows: `%LOCALAPPDATA%/QeriPlayer/Cache/`

The directory SHALL be created if it does not exist.

#### Scenario: Get cache directory
- **WHEN** `AppPaths::cacheDir()` is called
- **THEN** the returned path SHALL be a valid directory that exists

### Requirement: Temporary directory
The system SHALL provide `AppPaths::tempDir()` that returns a temporary directory for the application. The directory SHALL be created if it does not exist.

#### Scenario: Get temp directory
- **WHEN** `AppPaths::tempDir()` is called
- **THEN** the returned path SHALL exist and SHALL be writable

### Requirement: Safe file write
The system SHALL provide `FileUtils::writeFile(path, data)` that writes data atomically. Atomic write SHALL write to a temporary file in the same directory, then rename to the target path. This prevents partial writes on crash.

#### Scenario: Atomic write succeeds
- **WHEN** `FileUtils::writeFile("/tmp/test.txt", "hello")` is called
- **THEN** the file `/tmp/test.txt` SHALL contain "hello"

#### Scenario: Atomic write does not corrupt on failure
- **WHEN** `FileUtils::writeFile` encounters an I/O error during the temp-file write phase
- **THEN** the target file SHALL remain unchanged (or absent if it did not exist)

### Requirement: Safe file read
The system SHALL provide `FileUtils::readFile(path)` that returns the file contents as a `QByteArray`. If the file does not exist, it SHALL return an empty `QByteArray` and set an error flag.

#### Scenario: Read an existing file
- **WHEN** `FileUtils::readFile(path)` is called on a file containing "hello"
- **THEN** the returned QByteArray SHALL be "hello"

#### Scenario: Read a non-existent file
- **WHEN** `FileUtils::readFile(path)` is called on a path that does not exist
- **THEN** the returned QByteArray SHALL be empty and `FileUtils::lastError()` SHALL describe the missing file

### Requirement: Directory creation
The system SHALL provide `FileUtils::ensureDir(path)` that creates the directory and all parent directories if they do not exist. If the directory already exists, it SHALL be a no-op.

#### Scenario: Create nested directories
- **WHEN** `FileUtils::ensureDir("/tmp/a/b/c")` is called and only `/tmp/a` exists
- **THEN** directories `/tmp/a/b` and `/tmp/a/b/c` SHALL be created

#### Scenario: Directory already exists
- **WHEN** `FileUtils::ensureDir("/tmp/existing")` is called and the directory already exists
- **THEN** the call SHALL succeed without error

### Requirement: File watcher
The system SHALL provide a `FileWatcher` class that monitors a file or directory for changes. On change, it SHALL emit a `fileChanged(path)` signal.

#### Scenario: Watch a file for modification
- **WHEN** a `FileWatcher` is watching `/tmp/test.txt` and the file is written to
- **THEN** the `fileChanged` signal SHALL be emitted with the path `/tmp/test.txt`

#### Scenario: Stop watching
- **WHEN** `FileWatcher::stop()` is called
- **THEN** subsequent file changes SHALL NOT emit signals
