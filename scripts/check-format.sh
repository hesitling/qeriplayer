#!/usr/bin/env bash
# Check C++ formatting with clang-format. Exits non-zero on violation.
set -euo pipefail

base_ref="${1:-HEAD~1}"

# Validate base_ref exists; fall back to empty tree if unavailable
if ! git rev-parse --verify "$base_ref" >/dev/null 2>&1; then
  echo "Warning: base ref '$base_ref' not found, using empty tree." >&2
  base_ref="4b825dc642cb6eb9a060e54bf899d15f3f23e810"  # git empty tree
fi

files=$(git diff --name-only --diff-filter=ACMR "$base_ref" -- '*.h' '*.cpp')
if [ -z "$files" ]; then
  echo "No changed C++ files."
  exit 0
fi

echo "Checking formatting:"
echo "$files" | sed 's/^/  /'

diff=$(echo "$files" | xargs clang-format --dry-run --Werror --style=file 2>&1 || true)
if [ -n "$diff" ]; then
  echo ""
  echo "::error::Format violations found. Run 'just format' locally."
  echo "$diff"
  exit 1
fi

echo "All files formatted correctly."
