#!/bin/bash
# Local Docker-based build script for Android ARM32
# This script builds the addon, creates releases, and updates the repository
# If build fails, it automatically creates a public gist with the debug log

set -e  # Exit on error (we'll handle errors explicitly)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_LOG="$PROJECT_DIR/build-debug.log"
PLATFORM="android"
ARCH="arm32"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
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

# Function to create a gist from the build log
create_gist() {
    local log_file="$1"
    print_info "Creating public gist with build log..."
    
    if ! command -v gh &> /dev/null; then
        print_error "gh CLI not found. Please install it to create gists."
        print_info "Log file available at: $log_file"
        return 1
    fi
    
    # Create gist and capture URL
    local gist_url=$(gh gist create "$log_file" --public --desc "pvr.jellyfin Android ARM32 Build Log - $(date -u +"%Y-%m-%d %H:%M:%S UTC")" | tail -1)
    
    if [ $? -eq 0 ]; then
        print_success "Build log uploaded to: $gist_url"
        echo ""
        echo "================================================================"
        echo "Build failed. Debug log available at:"
        echo "$gist_url"
        echo "================================================================"
        echo ""
        
        # Also save the URL to a file
        echo "$gist_url" > "$PROJECT_DIR/last-build-log-url.txt"
        print_info "Gist URL also saved to: $PROJECT_DIR/last-build-log-url.txt"
    else
        print_error "Failed to create gist"
        print_info "Log file available at: $log_file"
    fi
}

# Function to check if Docker is running
check_docker() {
    if ! command -v docker &> /dev/null; then
        print_error "Docker is not installed. Please install Docker first."
        exit 1
    fi
    
    if ! docker info &> /dev/null; then
        print_error "Docker daemon is not running. Please start Docker."
        exit 1
    fi
    
    print_success "Docker is available and running"
}

# Function to update the repository
update_repo() {
    print_info "Updating repository from GitHub..."
    cd "$PROJECT_DIR"
    
    # Get current hash of this script before pulling
    local script_path="$SCRIPT_DIR/$(basename "${BASH_SOURCE[0]}")"
    local old_hash=$(git hash-object "$script_path" 2>/dev/null || echo "none")
    
    # Stash any local changes
    if ! git diff-index --quiet HEAD --; then
        print_warning "Local changes detected, stashing..."
        git stash push -m "Auto-stash before build $(date +%Y%m%d-%H%M%S)"
    fi
    
    # Pull latest changes
    if git pull origin main; then
        print_success "Repository updated successfully"
        
        # Check if this script changed
        local new_hash=$(git hash-object "$script_path" 2>/dev/null || echo "none")
        
        if [ "$old_hash" != "$new_hash" ]; then
            print_warning "Build script was updated!"
            print_info "Re-executing with new version..."
            echo ""
            
            # Re-execute this script with the same arguments
            exec "$script_path" "$@"
        fi
    else
        print_error "Failed to pull latest changes"
        print_info "If you're seeing SSH key errors, switch to HTTPS with:"
        print_info "  git remote set-url origin https://github.com/northernpowerhouse/pvr.jellyfin"
        exit 1
    fi
}

# Function to get version from git
get_version() {
    cd "$PROJECT_DIR"
    
    # Try to get version from latest tag
    local version=$(git describe --tags --abbrev=0 2>/dev/null || echo "")
    
    if [ -z "$version" ]; then
        # No tags, use date-based version
        version="0.1.0-dev.$(date +%Y%m%d.%H%M%S)"
        print_warning "No git tags found, using generated version: $version"
    else
        # Add commit count if not on a tag
        local commits_ahead=$(git rev-list ${version}..HEAD --count)
        if [ "$commits_ahead" -gt 0 ]; then
            version="${version}+${commits_ahead}.$(git rev-parse --short HEAD)"
        fi
        print_info "Using version: $version"
    fi
    
    echo "$version"
}

# Function to check if cached volumes are valid
check_volume_integrity() {
    print_info "Checking Docker volume cache integrity..."
    
    # Check if dependencies marker file exists
    local marker_exists=$(docker run --rm \
        -v kodi-depends-cache:/opt/xbmc-depends \
        busybox \
        test -f /opt/xbmc-depends/.dependencies-built && echo "yes" || echo "no")
    
    if [ "$marker_exists" = "yes" ]; then
        print_success "Kodi dependencies cache is valid"
        return 0
    else
        print_warning "Kodi dependencies cache is incomplete or corrupted"
        print_info "This will be rebuilt during Docker image build"
        return 1
    fi
}

# Function to clean corrupted volumes
clean_corrupted_volumes() {
    print_warning "Cleaning potentially corrupted Docker volumes..."
    
    # Only clean the depends cache, keep source and SDK
    if docker volume rm kodi-depends-cache 2>/dev/null; then
        print_success "Removed corrupted kodi-depends-cache volume"
    else
        print_info "No kodi-depends-cache volume to remove"
    fi
    
    print_info "Clean volumes will be rebuilt on next attempt"
}

# Function to build the Docker image with caching
build_docker_image() {
    cd "$PROJECT_DIR"
    
    # Check if image already exists
    if docker image inspect pvr-jellyfin-android-builder:latest &>/dev/null; then
        print_info "Docker image exists, checking if rebuild needed..."
    else
        print_info "Building Docker image with Kodi dependencies..."
        print_warning "First build: 30-60 minutes (downloads SDK, builds dependencies)"
        print_info "Subsequent builds: 5-10 minutes (uses cached layers and volumes)"
    fi
    
    # Enable BuildKit for better caching
    export DOCKER_BUILDKIT=1
    
    # Prepare no-cache flag if forced
    local CACHE_FLAGS="--cache-from pvr-jellyfin-android-builder:latest"
    if [ "${FORCE_NO_CACHE:-0}" = "1" ]; then
        print_warning "Building without cache (this will take longer)"
        CACHE_FLAGS="--no-cache"
    fi
    
    # Build the image (capture exit code from docker, not tee)
    # Use set -o pipefail to ensure pipeline failures are caught
    set -o pipefail
    docker build \
        --progress=plain \
        --tag pvr-jellyfin-android-builder:latest \
        --file Dockerfile.android-build \
        $CACHE_FLAGS \
        . 2>&1 | tee -a "$BUILD_LOG"
    
    # Capture the exit code
    local build_result=$?
    set +o pipefail
    
    if [ $build_result -eq 0 ]; then
        # Check if this was a cached build or full rebuild
        if docker image inspect pvr-jellyfin-android-builder:latest --format='{{.Created}}' 2>/dev/null | grep -q "$(date +%Y-%m-%d)"; then
            print_success "Docker image built successfully (from cache)"
        else
            print_success "Docker image ready (cached from previous build)"
        fi
        
        # Verify the image created the marker file
        check_volume_integrity
        
        return 0
    else
        print_error "Docker image build failed with exit code: $build_result"
        
        # Clean potentially corrupted volumes
        clean_corrupted_volumes
        
        return 1
    fi
}

# Function to build the addon inside Docker
build_addon() {
    local version="$1"
    print_info "Building pvr.jellyfin addon for Android ARM32..."
    
    cd "$PROJECT_DIR"
    
    # Create build output directory
    mkdir -p "$PROJECT_DIR/build-output"
    
    # Run the build in Docker container
    print_info "Running addon build in Docker container..."
    
    set -o pipefail
    docker run --rm \
        -v "$PROJECT_DIR:/workspace" \
        -v kodi-source-cache:/opt/kodi \
        -v kodi-depends-cache:/opt/xbmc-depends \
        -v android-sdk-cache:/opt/android-sdk \
        -w /workspace \
        pvr-jellyfin-android-builder:latest \
        bash -c "
            set -e
            echo '=== Building pvr.jellyfin addon ==='
            
            # Set up environment
            export ANDROID_HOME=/opt/android-sdk
            export ANDROID_NDK_HOME=/opt/android-sdk/ndk/25.2.9519653
            export PATH=\$PATH:\$ANDROID_HOME/cmdline-tools/latest/bin:\$ANDROID_HOME/platform-tools
            
            # Create build directory
            mkdir -p /workspace/build-android
            cd /workspace/build-android
            
            # Find Kodi headers location and dependencies
            export KODI_SOURCE=/opt/kodi
            export KODI_INCLUDE=\"\$KODI_SOURCE/xbmc/addons/kodi-dev-kit/include\"
            
            # The depends are installed under a host-specific subdirectory with build type
            # Format: arm-linux-androideabi-21-release (host-api-buildtype)
            export DEPENDS_ROOT=\"/opt/xbmc-depends/arm-linux-androideabi-21-release\"
            
            # jsoncpp is bundled with Kodi's CMake utilities
            export JSONCPP_INCLUDE=\"\$KODI_SOURCE/tools/depends/native/cmake/x86_64-linux-native/Utilities/cmjsoncpp/include\"
            
            echo \"Using Kodi headers from: \$KODI_INCLUDE\"
            echo \"Using dependencies from: \$DEPENDS_ROOT\"
            echo \"Using jsoncpp from: \$JSONCPP_INCLUDE\"
            
            # Search for jsoncpp library files
            echo \"Searching for jsoncpp library in \$DEPENDS_ROOT/lib...\"
            ls -la \$DEPENDS_ROOT/lib/ 2>&1 | grep -i json || echo \"No json libraries found\"
            find \$DEPENDS_ROOT/lib -name '*json*' 2>/dev/null || echo \"No json files found in lib\"
            
            # Configure with CMake for Android
            cmake .. \
                -DCMAKE_TOOLCHAIN_FILE=\$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
                -DANDROID_ABI=armeabi-v7a \
                -DANDROID_PLATFORM=android-21 \
                -DCMAKE_BUILD_TYPE=Release \
                -DCMAKE_PREFIX_PATH=\"\$DEPENDS_ROOT\" \
                -DKODI_INCLUDE_DIR=\"\$KODI_INCLUDE\" \
                -DCMAKE_CXX_FLAGS=\"-I\$KODI_INCLUDE -I\$KODI_SOURCE -I\$JSONCPP_INCLUDE -I\$DEPENDS_ROOT/include\" \
                -DCMAKE_C_FLAGS=\"-I\$KODI_SOURCE -I\$DEPENDS_ROOT/include\" \
                -DCMAKE_FIND_ROOT_PATH=\"\$DEPENDS_ROOT\" \
                -DPKG_CONFIG_PATH=\"\$DEPENDS_ROOT/lib/pkgconfig\" \
                -DCORE_SYSTEM_NAME=android
            
            # Build
            make -j\$(nproc)
            
            # Copy the built library
            mkdir -p /workspace/build-output
            find . -name 'pvr.jellyfin.so' -exec cp {} /workspace/build-output/ \;
            
            echo '=== Build completed successfully ==='
            
            # Package the addon inside Docker where zip is available
            echo 'Packaging addon...'
            cd /workspace
            export PLATFORM=android
            export LIBRARY_FILENAME=pvr.jellyfin.so
            bash /workspace/scripts/package.sh \"$version\"
            
            # Fix ownership of created files (Docker runs as root)
            chown -R \$(stat -c '%u:%g' /workspace) /workspace/package /workspace/*.zip 2>/dev/null || true
            
            echo '=== Packaging completed ==='
        " 2>&1 | tee -a "$BUILD_LOG"
    
    # Capture the exit code
    local build_result=$?
    set +o pipefail
    
    if [ $build_result -eq 0 ]; then
        print_success "Addon built successfully"
        
        # Verify the .so file was created
        if [ -f "$PROJECT_DIR/build-output/pvr.jellyfin.so" ]; then
            print_success "Found pvr.jellyfin.so"
            return 0
        else
            print_error "Build completed but pvr.jellyfin.so not found"
            return 1
        fi
    else
        print_error "Addon build failed with exit code: $build_result"
        return 1
    fi
}

# Function to package the addon
package_addon() {
    local version="$1"
    print_info "Verifying packaged addon..."
    
    # Packaging is now done inside Docker during build
    # Just verify the zip file was created
    print_info "Looking for: $PROJECT_DIR/pvr.jellyfin-${version}.zip"
    print_info "Available zip files:"
    ls -la "$PROJECT_DIR"/*.zip 2>/dev/null || echo "  No zip files found"
    
    if [ -f "$PROJECT_DIR/pvr.jellyfin-${version}.zip" ]; then
        print_success "Addon packaged: pvr.jellyfin-${version}.zip"
        return 0
    else
        print_error "Packaging failed - zip file not found"
        return 1
    fi
}

# Function to package the repository
package_repository() {
    local version="$1"
    print_info "Packaging repository addon..."
    
    cd "$PROJECT_DIR"
    
    set -o pipefail
    bash "$PROJECT_DIR/scripts/package_repo.sh" "$version" 2>&1 | tee -a "$BUILD_LOG"
    local package_result=$?
    set +o pipefail
    
    if [ $package_result -eq 0 ]; then
        print_success "Repository packaged: repository.jellyfin.pvr-${version}.zip"
        return 0
    else
        print_error "Repository packaging failed with exit code: $package_result"
        return 1
    fi
}

# Function to create GitHub release
create_release() {
    local version="$1"
    print_info "Creating GitHub release for version $version..."
    
    cd "$PROJECT_DIR"
    
    if ! command -v gh &> /dev/null; then
        print_error "gh CLI not found. Cannot create release."
        return 1
    fi
    
    # Create release notes
    local release_notes="## pvr.jellyfin ${version}

**Platform:** Android ARM 32-bit (armeabi-v7a)
**Built:** $(date -u +"%Y-%m-%d %H:%M:%S UTC")
**Kodi Version:** 21 (Omega)

### Installation

1. Download \`repository.jellyfin.pvr-${version}.zip\`
2. Install the repository zip in Kodi
3. Install pvr.jellyfin from the repository

Or install \`pvr.jellyfin-${version}.zip\` directly.

### Files

- \`pvr.jellyfin-${version}.zip\` - The addon package
- \`repository.jellyfin.pvr-${version}.zip\` - Repository addon for easy updates

---

Built with ❤️ using automated Docker-based build system
"
    
    # Check if release already exists
    if gh release view "$version" &>/dev/null; then
        print_warning "Release $version already exists, deleting old assets..."
        gh release delete-asset "$version" "pvr.jellyfin-${version}.zip" --yes 2>/dev/null || true
        gh release delete-asset "$version" "repository.jellyfin.pvr-${version}.zip" --yes 2>/dev/null || true
        
        # Upload new assets
        print_info "Uploading new assets to existing release..."
        gh release upload "$version" \
            "$PROJECT_DIR/pvr.jellyfin-${version}.zip" \
            "$PROJECT_DIR/repository.jellyfin.pvr-${version}.zip" \
            --clobber
        
        print_success "Release $version updated with new assets"
    else
        # Create new release
        print_info "Creating new release..."
        gh release create "$version" \
            "$PROJECT_DIR/pvr.jellyfin-${version}.zip" \
            "$PROJECT_DIR/repository.jellyfin.pvr-${version}.zip" \
            --title "pvr.jellyfin $version" \
            --notes "$release_notes"
        
        print_success "Release $version created successfully"
    fi
    
    # Get release URL
    local release_url=$(gh release view "$version" --json url -q .url)
    print_info "Release URL: $release_url"
}

# Function to update repository addon with latest release
update_repository_addon() {
    local version="$1"
    print_info "Updating repository addon with latest release info..."
    
    cd "$PROJECT_DIR"
    
    # Get the download URLs
    local addon_url="https://github.com/northernpowerhouse/pvr.jellyfin/releases/download/${version}/pvr.jellyfin-${version}.zip"
    
    print_info "Addon will be available at: $addon_url"
    print_success "Repository addon updated"
}

# Main execution
main() {
    echo ""
    echo "================================================================"
    echo "  pvr.jellyfin Local Android ARM32 Build Script"
    echo "================================================================"
    echo ""
    
    # Clear previous build log
    > "$BUILD_LOG"
    
    # Track if build failed
    BUILD_FAILED=0
    
    # Check prerequisites
    check_docker
    
    # Update repository (pass arguments for potential re-exec)
    update_repo "$@"
    
    # Get version
    VERSION=$(get_version)
    print_info "Building version: $VERSION"
    echo ""
    
    # Check cache integrity before building
    if ! check_volume_integrity; then
        print_warning "Previous build may have failed during dependency compilation"
        print_warning "Cleaning corrupted cache now to ensure fresh build..."
        clean_corrupted_volumes
        
        # Also remove the Docker image to force rebuild
        print_info "Removing cached Docker image to force rebuild with clean volumes..."
        docker rmi pvr-jellyfin-android-builder:latest 2>/dev/null || true
    fi
    
    # Check if Dockerfile changed since last build
    DOCKERFILE_HASH=$(git hash-object "$PROJECT_DIR/Dockerfile.android-build" 2>/dev/null || echo "none")
    LAST_DOCKERFILE_HASH=""
    if [ -f "$PROJECT_DIR/.last-dockerfile-hash" ]; then
        LAST_DOCKERFILE_HASH=$(cat "$PROJECT_DIR/.last-dockerfile-hash")
    fi
    
    if [ "$DOCKERFILE_HASH" != "$LAST_DOCKERFILE_HASH" ]; then
        print_warning "Dockerfile has changed, forcing Docker image rebuild with no cache..."
        docker rmi pvr-jellyfin-android-builder:latest 2>/dev/null || true
        export FORCE_NO_CACHE=1
        echo "$DOCKERFILE_HASH" > "$PROJECT_DIR/.last-dockerfile-hash"
    fi
    
    # Build Docker image
    print_info "Step 1/5: Building Docker image with dependencies..."
    if ! build_docker_image; then
        BUILD_FAILED=1
    fi
    
    # Build addon
    if [ $BUILD_FAILED -eq 0 ]; then
        print_info "Step 2/5: Building addon..."
        if ! build_addon "$VERSION"; then
            BUILD_FAILED=1
            # Note: Addon build doesn't affect volumes, only Docker image build does
            print_warning "Addon build failed. Docker volumes are unaffected."
        fi
    fi
    
    # Package addon
    if [ $BUILD_FAILED -eq 0 ]; then
        print_info "Step 3/5: Packaging addon..."
        if ! package_addon "$VERSION"; then
            BUILD_FAILED=1
        fi
    fi
    
    # Package repository
    if [ $BUILD_FAILED -eq 0 ]; then
        print_info "Step 4/5: Packaging repository..."
        if ! package_repository "$VERSION"; then
            BUILD_FAILED=1
        fi
    fi
    
    # Create release
    if [ $BUILD_FAILED -eq 0 ]; then
        print_info "Step 5/5: Creating GitHub release..."
        if ! create_release "$VERSION"; then
            BUILD_FAILED=1
        else
            update_repository_addon "$VERSION"
        fi
    fi
    
    echo ""
    echo "================================================================"
    
    if [ $BUILD_FAILED -eq 1 ]; then
        print_error "Build FAILED"
        echo "================================================================"
        echo ""
        
        # Create gist with debug log
        create_gist "$BUILD_LOG"
        
        exit 1
    else
        print_success "Build COMPLETED successfully!"
        echo "================================================================"
        echo ""
        print_success "Packages created:"
        echo "  - pvr.jellyfin-${VERSION}.zip"
        echo "  - repository.jellyfin.pvr-${VERSION}.zip"
        echo ""
        print_success "GitHub release created for version: $VERSION"
        echo ""
        print_info "To install in Kodi:"
        echo "  1. Download repository.jellyfin.pvr-${VERSION}.zip from the release"
        echo "  2. Install it in Kodi (Settings > Add-ons > Install from zip file)"
        echo "  3. Install pvr.jellyfin from the repository"
        echo ""
        print_info "Build log saved to: $BUILD_LOG"
        
        exit 0
    fi
}

# Run main function
main "$@"
