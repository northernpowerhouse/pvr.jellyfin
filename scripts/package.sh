#!/bin/bash
# Package script for pvr.jellyfin addon

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
VERSION="${1:-1.0.0}"

# Allow overriding platform and library filename from env (useful for CI)
PLATFORM="${PLATFORM:-all}"
LIBRARY_FILENAME="${LIBRARY_FILENAME:-pvr.jellyfin.so}"

echo "Packaging pvr.jellyfin addon version $VERSION..."

# Create package directory
PACKAGE_DIR="$PROJECT_DIR/package"
ADDON_DIR="$PACKAGE_DIR/pvr.jellyfin"

rm -rf "$PACKAGE_DIR"
mkdir -p "$ADDON_DIR"

# Copy files
echo "Copying addon files..."
cp -r "$PROJECT_DIR/resources" "$ADDON_DIR/"
cp "$PROJECT_DIR/LICENSE" "$ADDON_DIR/"
cp "$PROJECT_DIR/README.md" "$ADDON_DIR/"

# Copy the built library file
if [ -f "$PROJECT_DIR/build-output/$LIBRARY_FILENAME" ]; then
    echo "Copying built library: $LIBRARY_FILENAME"
    cp "$PROJECT_DIR/build-output/$LIBRARY_FILENAME" "$ADDON_DIR/"
elif [ -f "$PROJECT_DIR/build-android/$LIBRARY_FILENAME" ]; then
    echo "Copying built library from build-android: $LIBRARY_FILENAME"
    cp "$PROJECT_DIR/build-android/$LIBRARY_FILENAME" "$ADDON_DIR/"
else
    echo "ERROR: Built library not found at $PROJECT_DIR/build-output/$LIBRARY_FILENAME or $PROJECT_DIR/build-android/$LIBRARY_FILENAME"
    exit 1
fi

# Process addon.xml.in
echo "Processing addon.xml... (platform=$PLATFORM, library=$LIBRARY_FILENAME)"
sed "s/@ADDON_DEPENDS@/<import addon=\"xbmc.pvr\" version=\"8.2.0\"\/\>/g" \
    "$PROJECT_DIR/addon.xml.in" | \
sed "s/@PLATFORM@/${PLATFORM}/g" | \
sed "s/@LIBRARY_FILENAME@/${LIBRARY_FILENAME}/g" \
    > "$ADDON_DIR/addon.xml"

# Create zip
cd "$PACKAGE_DIR"
ZIP_NAME="pvr.jellyfin-$VERSION.zip"
echo "Creating $ZIP_NAME..."
zip -r "$PROJECT_DIR/$ZIP_NAME" pvr.jellyfin

echo "Package created: $PROJECT_DIR/$ZIP_NAME"
