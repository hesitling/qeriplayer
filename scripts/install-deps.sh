#!/usr/bin/env bash
# Install system dependencies for building QeriPlayer Qt on Ubuntu.
# Qt is installed separately via jurplel/install-qt-action in CI.
set -euo pipefail

sudo apt-get update
sudo apt-get install -y --no-install-recommends \
  build-essential cmake ninja-build pkg-config \
  libspdlog-dev libsqlite3-dev libssl-dev \
  libgl1-mesa-dev
