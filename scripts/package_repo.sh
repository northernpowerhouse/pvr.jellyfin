#!/bin/bash
# Package the repository addon for distribution

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
VERSION="${1:-1.0.0}"

echo "Packaging repository addon version $VERSION..."

# Create package directory
PACKAGE_DIR="$PROJECT_DIR/package-repo"
REPO_DIR="$PACKAGE_DIR/repository.jellyfin.pvr"

rm -rf "$PACKAGE_DIR"
mkdir -p "$REPO_DIR"

# Copy repository addon files
echo "Copying repository addon files..."
cp -r "$PROJECT_DIR/repository/repository.jellyfin.pvr/"* "$REPO_DIR/"

# Update version in addon.xml
# Use | as delimiter to avoid issues with special characters in version string
sed -i "s|version=\"[^\"]*\"|version=\"$VERSION\"|" "$REPO_DIR/addon.xml"

# Create icon placeholder if it doesn't exist
if [ ! -f "$REPO_DIR/icon.png" ]; then
  echo "Creating icon placeholder..."
  # Create a simple placeholder text file
  cat > "$REPO_DIR/icon.png.placeholder" << 'EOF'
Replace this file with a 256x256 PNG icon representing the Jellyfin PVR repository.

Suggested design:
- Jellyfin logo
- TV/antenna icon
- Kodi logo (if licensing permits)
- Repository/package symbol
EOF
fi

# Create README for the repository addon
cat > "$REPO_DIR/README.md" << 'EOF'
# Jellyfin PVR Repository Addon

This is the repository addon that provides easy installation and automatic updates for the Jellyfin PVR Client.

## Installation

1. Download the repository zip file
2. In Kodi, go to Settings > Add-ons > Install from zip file
3. Select the downloaded repository.jellyfin.pvr.zip
4. Once installed, you can install the Jellyfin PVR Client from:
   - Settings > Add-ons > Install from repository > Jellyfin PVR Repository

## What This Does

This repository addon tells Kodi where to find and download the Jellyfin PVR Client addon and its updates.

## Support

For issues, visit: https://github.com/northernpowerhouse/pvr.jellyfin/issues
EOF

# Create zip
cd "$PACKAGE_DIR"
ZIP_NAME="repository.jellyfin.pvr-$VERSION.zip"
echo "Creating $ZIP_NAME..."
zip -r "$PROJECT_DIR/$ZIP_NAME" repository.jellyfin.pvr

echo "Repository package created: $PROJECT_DIR/$ZIP_NAME"
echo ""
echo "To distribute:"
echo "1. Upload this zip to GitHub releases"
echo "2. Users install this zip first"
echo "3. Then they can install pvr.jellyfin from the repository"
