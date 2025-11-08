#!/bin/bash
# Upload Jellyfin log to GitHub repository and delete local copy
# Usage: ./scripts/upload-jellyfin-log.sh [YYYYMMDD]
#        If no date provided, uses today's date

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Get date from argument or use today
if [ -n "$1" ]; then
    DATE="$1"
else
    DATE=$(date +%Y%m%d)
fi

# Validate date format
if ! [[ "$DATE" =~ ^[0-9]{8}$ ]]; then
    print_error "Invalid date format. Expected YYYYMMDD, got: $DATE"
    exit 1
fi

# Get current time for unique filename
TIMESTAMP=$(date +%Y%m%d%H%M)

# Source and destination paths
SOURCE_LOG="/docker/jellyfinpr/log/log_${DATE}.log"
DEST_NAME="jellyfinlog_${TIMESTAMP}.log"
DEST_DIR="$PROJECT_DIR/devlog"
DEST_PATH="$DEST_DIR/$DEST_NAME"

echo ""
echo "================================================================"
echo "  Jellyfin Log Upload Script"
echo "================================================================"
echo ""
print_info "Date: $DATE"
print_info "Timestamp: $TIMESTAMP"
print_info "Source: $SOURCE_LOG"
print_info "Destination: devlog/$DEST_NAME"
echo ""

# Check if source log exists
if [ ! -f "$SOURCE_LOG" ]; then
    print_error "Log file not found: $SOURCE_LOG"
    exit 1
fi

print_success "Found log file: $SOURCE_LOG"

# Get file size for info
FILE_SIZE=$(du -h "$SOURCE_LOG" | cut -f1)
print_info "File size: $FILE_SIZE"

# Create devlog directory if it doesn't exist
mkdir -p "$DEST_DIR"

# Copy the log file with new name
print_info "Copying log file..."
cp "$SOURCE_LOG" "$DEST_PATH"
print_success "Log copied to: $DEST_PATH"

# Add to git
cd "$PROJECT_DIR"
git add "$DEST_PATH"

# Commit
print_info "Committing to repository..."
if git commit -m "Add Jellyfin log for $DATE at $(date +%H:%M)"; then
    print_success "Log committed"
    
    # Push to GitHub
    print_info "Pushing to GitHub..."
    if git push origin main; then
        print_success "Log uploaded to GitHub"
        
        # Delete local source file
        print_info "Deleting local log file..."
        if rm "$SOURCE_LOG"; then
            print_success "Local log deleted: $SOURCE_LOG"
        else
            print_warning "Failed to delete local log (may need sudo)"
            print_info "You can manually delete it with: sudo rm $SOURCE_LOG"
        fi
        
        echo ""
        echo "================================================================"
        print_success "Log upload completed successfully!"
        echo "================================================================"
        echo ""
        print_info "Log available at:"
        echo "  https://github.com/northernpowerhouse/pvr.jellyfin/blob/main/devlog/$DEST_NAME"
        
    else
        print_error "Failed to push to GitHub"
        exit 1
    fi
else
    print_warning "No changes to commit (log may already be uploaded)"
    
    # Still try to delete local file
    print_info "Deleting local log file..."
    if rm "$SOURCE_LOG"; then
        print_success "Local log deleted: $SOURCE_LOG"
    else
        print_warning "Failed to delete local log (may need sudo)"
    fi
fi

exit 0
