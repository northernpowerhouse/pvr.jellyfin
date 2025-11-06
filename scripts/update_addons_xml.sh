#!/bin/bash
# Generate addons.xml and addons.xml.md5 files for the repository
# This includes both the pvr.jellyfin addon and the repository addon itself

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
REPO_DIR="$PROJECT_DIR/repository"
ADDON_XML="$PROJECT_DIR/addon.xml.in"
REPO_ADDON_XML="$REPO_DIR/repository.jellyfin.pvr/addon.xml"
OUTPUT_XML="$REPO_DIR/addons.xml"
OUTPUT_MD5="$REPO_DIR/addons.xml.md5"

# Get version if provided, otherwise use placeholder
VERSION="${1:-1.0.0}"

echo "Generating addons.xml for version: $VERSION"

# Start the addons.xml file
cat > "$OUTPUT_XML" << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<addons>
EOF

# Add pvr.jellyfin addon
echo "  <!-- pvr.jellyfin addon -->" >> "$OUTPUT_XML"

# Parse addon.xml.in and replace version and template variables
while IFS= read -r line; do
    if [[ "$line" =~ ^\<\?xml ]]; then
        continue
    elif [[ "$line" =~ version= ]]; then
        # Replace version anywhere it appears
        line=$(echo "$line" | sed "s|version=\"[^\"]*\"|version=\"$VERSION\"|")
        echo "  $line" >> "$OUTPUT_XML"
    elif [[ "$line" =~ \</addon\> ]]; then
        echo "  </addon>" >> "$OUTPUT_XML"
        break
    elif [[ ! -z "$line" ]]; then
        # Replace template variables with actual values for Android
        line=$(echo "$line" | sed 's|@ADDON_DEPENDS@|<import addon="kodi.binary.global.main" version="2.0.0"/>\n    <import addon="kodi.binary.instance.pvr" version="8.2.0"/>|')
        line=$(echo "$line" | sed 's|@PLATFORM@|android|g')
        line=$(echo "$line" | sed 's|@LIBRARY_FILENAME@|pvr.jellyfin.so|')
        line=$(echo "$line" | sed 's|library_@PLATFORM@|library_android|')
        echo "  $line" >> "$OUTPUT_XML"
    fi
done < "$ADDON_XML"

echo "" >> "$OUTPUT_XML"

# Add repository addon (so it can self-update)
echo "  <!-- repository.jellyfin.pvr addon (self-update) -->" >> "$OUTPUT_XML"
while IFS= read -r line; do
    if [[ "$line" =~ ^\<\?xml ]]; then
        continue
    elif [[ ! -z "$line" ]]; then
        echo "  $line" >> "$OUTPUT_XML"
    fi
done < "$REPO_ADDON_XML"

echo "" >> "$OUTPUT_XML"

# Close the addons.xml file
echo "</addons>" >> "$OUTPUT_XML"

echo "Generated: $OUTPUT_XML"

# Generate MD5 checksum
cd "$REPO_DIR"
md5sum addons.xml | cut -d' ' -f1 > addons.xml.md5

echo "Generated: $OUTPUT_MD5"
echo "MD5: $(cat $OUTPUT_MD5)"
echo ""
echo "✓ Repository metadata updated successfully"
echo ""
echo "Files created:"
echo "  - repository/addons.xml (describes both addons)"
echo "  - repository/addons.xml.md5 (checksum)"
echo ""
echo "These files must be committed to git so they're available at:"
echo "  https://raw.githubusercontent.com/northernpowerhouse/pvr.jellyfin/main/repository/addons.xml"
echo "  https://raw.githubusercontent.com/northernpowerhouse/pvr.jellyfin/main/repository/addons.xml.md5"
echo ""
echo "⚠️  IMPORTANT: Commit and push to enable automatic updates:"
echo ""
echo "  git add repository/addons.xml repository/addons.xml.md5"
echo "  git commit -m 'Update repository metadata for version $VERSION'"
echo "  git push"
echo ""
