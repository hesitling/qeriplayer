## ADDED Requirements

### Requirement: Sidebar vertical alignment
`Sidebar.qml` SHALL display navigation items with consistent vertical alignment. All items (Home, Search, Library, Settings) SHALL be vertically centered within their containers, ensuring uniform visual positioning across the sidebar.

#### Scenario: Consistent item positioning
- **WHEN** the application window is displayed
- **THEN** all navigation items in the sidebar SHALL have their text vertically aligned at the same position relative to their container height

#### Scenario: Settings item alignment
- **WHEN** the Settings navigation item is rendered
- **THEN** its text SHALL be vertically centered at the same height as other navigation items (Home, Search, Library)

### Requirement: Sidebar vertical expansion
`Sidebar.qml` SHALL expand vertically to fill available space. The sidebar content SHALL be distributed evenly across the available height, preventing items from appearing cramped or misaligned.

#### Scenario: Sidebar fills available height
- **WHEN** the application window is resized vertically
- **THEN** the sidebar SHALL expand to fill the full height of the content area

#### Scenario: Even distribution of items
- **WHEN** the sidebar has more vertical space than needed for items
- **THEN** items SHALL be distributed with consistent spacing, not clustered at the top
