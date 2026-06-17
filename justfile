# justfile — NeriPlayer Qt task runner
#
# Usage:
#   just              # show available commands
#   just build        # configure + build
#   just test         # build + run all tests
#   just test Foo     # run tests matching "Foo"
#   just format       # format changed files
#   just check        # format check (CI-friendly, no writes)
#   just ci           # build + test + format check

# Default build directory (override with: just build_dir=/tmp/neriplayer-build build)
# Or set JUST_BUILD_DIR env var for the session.
build_dir := env_var_or_default("JUST_BUILD_DIR", "build")

# ─── Build ────────────────────────────────────────────────────────

# Configure (first time or after CMakeLists.txt changes)
[group('build')]
configure:
    cmake -B {{build_dir}} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build the project
[group('build')]
build: configure
    cmake --build {{build_dir}} -j$(nproc)

# Clean build directory
[group('build')]
clean:
    rm -rf {{build_dir}}

# Full rebuild
[group('build')]
rebuild: clean build

# ─── Test ─────────────────────────────────────────────────────────

# Run all tests (builds first)
[group('test')]
test: build
    cd {{build_dir}} && ctest --output-on-failure -j$(nproc)

# Run tests matching a pattern: just test-repo Song
[group('test')]
test-match name: build
    cd {{build_dir}} && ctest --output-on-failure -R '{{name}}'

# Run a single test binary directly: just test-run TestDatabase
[group('test')]
test-run name: build
    ./{{build_dir}}/{{name}}

# List all registered test targets
[group('test')]
test-list:
    cd {{build_dir}} && ctest -N

# ─── Format ───────────────────────────────────────────────────────

# Format all staged + changed source files (in-place)
[group('format')]
format:
    #!/usr/bin/env bash
    set -euo pipefail
    files=$(git diff --name-only --diff-filter=ACMR HEAD -- '*.h' '*.cpp' || true)
    if [ -z "$files" ]; then
        echo "No changed files to format."
        exit 0
    fi
    echo "Formatting:"
    echo "$files" | sed 's/^/  /'
    echo "$files" | xargs clang-format -i --style=file

# Check formatting (no writes, exits non-zero on violation)
[group('format')]
check:
    #!/usr/bin/env bash
    set -euo pipefail
    files=$(git diff --name-only --diff-filter=ACMR HEAD -- '*.h' '*.cpp' || true)
    if [ -z "$files" ]; then
        echo "No changed files to check."
        exit 0
    fi
    diff=$(echo "$files" | xargs clang-format --dry-run --Werror --style=file 2>&1 || true)
    if [ -n "$diff" ]; then
        echo "Format violations found:"
        echo "$diff"
        echo ""
        echo "Run 'just format' to fix."
        exit 1
    fi
    echo "All files formatted correctly."

# Format a specific file: just format-file src/core/database/DatabaseManager.cpp
[group('format')]
format-file file:
    clang-format -i --style=file {{file}}

# ─── CI Pipeline ──────────────────────────────────────────────────

# Full CI check: build → test → format check
[group('ci')]
ci: build test check

# ─── Utilities ────────────────────────────────────────────────────

# Open compile_commands.json in build/ (for IDEs)
[group('util')]
link-compile-commands:
    ln -sf {{build_dir}}/compile_commands.json compile_commands.json

# Count source files
[group('util')]
stats:
    @echo "Source files:  $(find src -name '*.cpp' -o -name '*.h' | wc -l)"
    @echo "Test files:    $(find tests -name '*.cpp' | wc -l)"
    @echo "Lines of code: $(find src tests -name '*.cpp' -o -name '*.h' | xargs wc -l | tail -1 | awk '{print $1}')"
