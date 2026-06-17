## Purpose

Defines the settings repository for persisting application preferences as key-value pairs. Provides typed accessors for boolean and integer settings with default fallbacks.

## Requirements

### Requirement: Get setting value
The system SHALL provide a method to retrieve a setting value by key. If the key does not exist, an empty optional SHALL be returned.

#### Scenario: Get existing setting
- **WHEN** `get("audio_quality")` is called and the key exists with value "high"
- **THEN** the result SHALL be `QString("high")`

#### Scenario: Get non-existent setting
- **WHEN** `get("nonexistent")` is called
- **THEN** an empty optional SHALL be returned

### Requirement: Set setting value
The system SHALL provide a method to set a setting value by key. If the key already exists, the value SHALL be updated (upsert behavior).

#### Scenario: Set new setting
- **WHEN** `set("audio_quality", "high")` is called
- **THEN** the `settings` table SHALL contain a row with `key="audio_quality"` and `value="high"`

#### Scenario: Update existing setting
- **WHEN** `set("audio_quality", "lossless")` is called after setting it to "high"
- **THEN** the value SHALL be "lossless"

### Requirement: Remove setting
The system SHALL provide a method to delete a setting by key. If the key does not exist, the operation SHALL be a no-op.

#### Scenario: Remove existing setting
- **WHEN** `remove("audio_quality")` is called and the key exists
- **THEN** the key SHALL no longer exist in the `settings` table

#### Scenario: Remove non-existent setting
- **WHEN** `remove("nonexistent")` is called
- **THEN** the operation SHALL complete without error

### Requirement: Get all settings
The system SHALL provide a method to retrieve all settings as a `QVariantMap` (key→value).

#### Scenario: Get all settings
- **WHEN** `getAll()` is called and 3 settings exist
- **THEN** the result SHALL be a map with 3 entries

### Requirement: Typed setting accessors
The system SHALL provide convenience methods `getBool()` and `getInt()` that retrieve a setting and convert it to the requested type with a default fallback.

#### Scenario: Get boolean setting
- **WHEN** `getBool("shuffle_enabled", false)` is called and the value is "true"
- **THEN** the result SHALL be `true`

#### Scenario: Get boolean with default
- **WHEN** `getBool("nonexistent", true)` is called
- **THEN** the result SHALL be `true`

#### Scenario: Get integer setting
- **WHEN** `getInt("volume", 50)` is called and the value is "80"
- **THEN** the result SHALL be `80`

#### Scenario: Get integer with default
- **WHEN** `getInt("nonexistent", 42)` is called
- **THEN** the result SHALL be `42`
