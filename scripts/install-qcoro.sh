#!/usr/bin/env bash
# Clone QCoro into lib/qcoro for the CMake local-build fallback.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

qcoro_version="${1:-v0.13.0}"
target_dir="$REPO_ROOT/lib/qcoro"

if [ -d "$target_dir" ]; then
  echo "QCoro already present at $target_dir, skipping clone."
  exit 0
fi

echo "Cloning QCoro $qcoro_version ..."
git clone --depth 1 --branch "$qcoro_version" \
  https://github.com/danvratil/qcoro.git "$target_dir"
