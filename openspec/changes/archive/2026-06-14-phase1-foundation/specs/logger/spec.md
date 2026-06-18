## ADDED Requirements

### Requirement: Logger initialization
The system SHALL provide a `Logger` class that initializes spdlog with a file sink and a console sink. Initialization SHALL be called once during application startup.

#### Scenario: Initialize logger
- **WHEN** `Logger::initialize(config)` is called with a valid configuration
- **THEN** log messages at the configured level and above SHALL be written to both file and console

#### Scenario: Double initialization
- **WHEN** `Logger::initialize()` is called a second time
- **THEN** the second call SHALL be a no-op and the original configuration SHALL be preserved

### Requirement: Named loggers
The system SHALL support creating named loggers via `Logger::get(name)`. Each named logger SHALL prefix its output with the logger name. Named loggers SHALL share the same sinks.

#### Scenario: Create a named logger
- **WHEN** `Logger::get("network")` is called
- **THEN** the returned logger SHALL prefix all messages with "network"

#### Scenario: Get the same logger twice
- **WHEN** `Logger::get("network")` is called twice
- **THEN** the same logger instance SHALL be returned both times

### Requirement: Log levels
The system SHALL support the following log levels in order of severity: `trace`, `debug`, `info`, `warn`, `error`, `fatal`. The global level SHALL be configurable. Messages below the global level SHALL be discarded.

#### Scenario: Set log level to info
- **WHEN** the log level is set to `info` and a `debug` message is logged
- **THEN** the message SHALL NOT appear in any sink

#### Scenario: Log at error level
- **WHEN** the log level is set to `info` and an `error` message is logged
- **THEN** the message SHALL appear in both file and console sinks

### Requirement: File sink with rotation
The file sink SHALL create daily log files in the configured log directory. Files SHALL be named with the pattern `qeriplayer-YYYY-MM-DD.log`. Old log files SHALL be retained for 7 days and then deleted.

#### Scenario: Log file creation
- **WHEN** the application starts and logs a message
- **THEN** a log file named `qeriplayer-YYYY-MM-DD.log` for the current date SHALL exist in the log directory

#### Scenario: Old log cleanup
- **WHEN** log files older than 7 days exist
- **THEN** they SHALL be deleted on application startup

### Requirement: Console sink with color
The console sink SHALL output colored log messages when the terminal supports ANSI colors. Color SHALL be disabled when output is redirected to a pipe or file.

#### Scenario: Colored output in terminal
- **WHEN** the application runs in a terminal that supports ANSI
- **THEN** log level labels (ERROR, WARN, etc.) SHALL be colored

#### Scenario: Plain output when piped
- **WHEN** the application output is piped to a file
- **THEN** log messages SHALL be plain text without ANSI escape codes

### Requirement: Log format
The log format SHALL be: `[YYYY-MM-DD HH:MM:SS.mmm] [level] [logger] message`. The timestamp SHALL use the local timezone.

#### Scenario: Format a log message
- **WHEN** an info message "connected" is logged from the "network" logger
- **THEN** the output SHALL match the pattern `[2024-01-15 10:30:45.123] [info] [network] connected`

### Requirement: Runtime log level change
The system SHALL allow changing the log level at runtime without restarting the application.

#### Scenario: Change level from info to debug
- **WHEN** the log level is changed from `info` to `debug` at runtime
- **THEN** subsequent `debug` messages SHALL be logged and prior `debug` messages that were discarded SHALL not be recovered
