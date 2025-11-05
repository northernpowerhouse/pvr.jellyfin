#!/bin/bash
# Build script for pvr.jellyfin addon

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

echo "Building pvr.jellyfin addon..."

# Clean previous build
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Run CMake
echo "Running CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo "Building..."
cmake --build . --config Release

echo "Build complete!"
echo "Build artifacts are in: $BUILD_DIR"
