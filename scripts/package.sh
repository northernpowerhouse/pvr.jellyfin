#!/bin/bash
# Package script for pvr.jellyfin addon

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
VERSION="${1:-1.0.0}"

echo "Packaging pvr.jellyfin addon version $VERSION..."

# Create package directory
PACKAGE_DIR="$PROJECT_DIR/package"
ADDON_DIR="$PACKAGE_DIR/pvr.jellyfin"

rm -rf "$PACKAGE_DIR"
mkdir -p "$ADDON_DIR"

# Copy files
echo "Copying addon files..."
cp -r "$PROJECT_DIR/src" "$ADDON_DIR/"
cp -r "$PROJECT_DIR/resources" "$ADDON_DIR/"
cp "$PROJECT_DIR/LICENSE" "$ADDON_DIR/"
cp "$PROJECT_DIR/README.md" "$ADDON_DIR/"
cp "$PROJECT_DIR/CMakeLists.txt" "$ADDON_DIR/"

# Process addon.xml.in
echo "Processing addon.xml..."
sed "s/@ADDON_DEPENDS@/<import addon=\"xbmc.pvr\" version=\"8.2.0\"\/>/g" \
    "$PROJECT_DIR/addon.xml.in" | \
sed "s/@PLATFORM@/all/g" | \
sed "s/@LIBRARY_FILENAME@/pvr.jellyfin.so/g" \
    > "$ADDON_DIR/addon.xml"

# Create zip
cd "$PACKAGE_DIR"
ZIP_NAME="pvr.jellyfin-$VERSION.zip"
echo "Creating $ZIP_NAME..."
zip -r "$PROJECT_DIR/$ZIP_NAME" pvr.jellyfin

echo "Package created: $PROJECT_DIR/$ZIP_NAME"
