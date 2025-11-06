# Local Android Build Script

## Overview

The `build-local-android.sh` script provides a complete Docker-based build system for building the pvr.jellyfin addon for Android ARM 32-bit on your local machine. It includes intelligent caching, automatic error reporting, and GitHub release creation.

## Features

- ✅ **Docker-based building** - Consistent build environment
- ✅ **Intelligent caching** - Kodi source, dependencies, and Android SDK cached in Docker volumes
- ✅ **Auto-update** - Pulls latest code from GitHub before building
- ✅ **Error reporting** - Automatically creates public gist with debug logs on failure
- ✅ **Release creation** - Creates GitHub releases with both addon and repository packages
- ✅ **Progress tracking** - Colored output with clear status messages

## Prerequisites

### Required Software

1. **Docker** - Install from [docker.com](https://www.docker.com/)
   ```bash
   # Check if Docker is installed and running
   docker --version
   docker info
   ```

2. **GitHub CLI** - Install from [cli.github.com](https://cli.github.com/)
   ```bash
   # Check if gh CLI is installed
   gh --version
   
   # Login to GitHub (if not already)
   gh auth login
   ```

3. **Git** - For repository management
   ```bash
   git --version
   ```

### System Requirements

- **Disk Space**: ~15-20 GB for Docker images and caches (first build)
- **RAM**: 4 GB minimum, 8 GB recommended
- **CPU**: Multi-core recommended for faster builds
- **OS**: Linux, macOS, or Windows with WSL2

## Usage

### Basic Usage

Simply run the script from anywhere:

```bash
./scripts/build-local-android.sh
```

The script will:
1. ✅ Check Docker is running
2. ✅ Pull latest code from GitHub (`git pull`)
3. ✅ Determine version from git tags or generate one
4. ✅ Build Docker image with all dependencies (cached)
5. ✅ Build the addon
6. ✅ Package the addon and repository
7. ✅ Create GitHub release
8. ✅ Upload packages to the release

### First Run

The first run will take **30-60 minutes** because it needs to:
- Download Android SDK/NDK (~1 GB)
- Clone Kodi source code (~500 MB)
- Build all Kodi dependencies for Android ARM

**Subsequent runs** take only **5-10 minutes** because everything is cached!

### Output

On success:
```
================================================================
  pvr.jellyfin Local Android ARM32 Build Script
================================================================

[SUCCESS] Docker is available and running
[INFO] Updating repository from GitHub...
[SUCCESS] Repository updated successfully
[INFO] Using version: 1.0.0
[INFO] Building version: 1.0.0

[INFO] Step 1/5: Building Docker image with dependencies...
[SUCCESS] Docker image built successfully

[INFO] Step 2/5: Building addon...
[SUCCESS] Addon built successfully
[SUCCESS] Found pvr.jellyfin.so

[INFO] Step 3/5: Packaging addon...
[SUCCESS] Addon packaged: pvr.jellyfin-1.0.0.zip

[INFO] Step 4/5: Packaging repository...
[SUCCESS] Repository packaged: repository.jellyfin.pvr-1.0.0.zip

[INFO] Step 5/5: Creating GitHub release...
[SUCCESS] Release 1.0.0 created successfully
[INFO] Release URL: https://github.com/northernpowerhouse/pvr.jellyfin/releases/tag/1.0.0

================================================================
[SUCCESS] Build COMPLETED successfully!
================================================================

[SUCCESS] Packages created:
  - pvr.jellyfin-1.0.0.zip
  - repository.jellyfin.pvr-1.0.0.zip

[SUCCESS] GitHub release created for version: 1.0.0

[INFO] To install in Kodi:
  1. Download repository.jellyfin.pvr-1.0.0.zip from the release
  2. Install it in Kodi (Settings > Add-ons > Install from zip file)
  3. Install pvr.jellyfin from the repository
```

On failure:
```
[ERROR] Build FAILED
================================================================

[INFO] Creating public gist with build log...
[SUCCESS] Build log uploaded to: https://gist.github.com/...

================================================================
Build failed. Debug log available at:
https://gist.github.com/...
================================================================

[INFO] Gist URL also saved to: last-build-log-url.txt
```

## Understanding the Caching

The script uses Docker volumes to cache data between builds:

### Docker Volumes

- `kodi-source-cache` - Kodi source code (~500 MB)
- `kodi-depends-cache` - Built Kodi dependencies (~2 GB)
- `android-sdk-cache` - Android SDK/NDK (~3 GB)

### View Cache Status

```bash
# List Docker volumes
docker volume ls | grep kodi

# Inspect a volume
docker volume inspect kodi-source-cache

# Check disk usage
docker system df -v
```

### Clear Cache

If you need to rebuild from scratch:

```bash
# Remove all caches
docker volume rm kodi-source-cache kodi-depends-cache android-sdk-cache

# Or remove everything Docker-related
docker system prune -a --volumes
```

## Version Management

The script automatically determines the version:

### Using Git Tags

```bash
# Create a version tag
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin v1.0.0

# Script will use: 1.0.0
./scripts/build-local-android.sh
```

### Between Tags

If you have commits after the last tag:

```bash
# Last tag: v1.0.0, with 5 commits since
# Script will use: 1.0.0+5.abc1234
```

### No Tags

If no tags exist:

```bash
# Script will generate: 0.1.0-dev.20250106.123456
```

## Troubleshooting

### Docker Issues

**Problem**: `Docker daemon is not running`

```bash
# On Linux
sudo systemctl start docker

# On macOS
# Start Docker Desktop application

# On Windows
# Start Docker Desktop
```

**Problem**: `permission denied while trying to connect to Docker`

```bash
# Add your user to docker group (Linux)
sudo usermod -aG docker $USER
# Log out and back in
```

### Build Failures

If the build fails, the script automatically:
1. Creates a public gist with the full debug log
2. Prints the gist URL
3. Saves the URL to `last-build-log-url.txt`

Share the gist URL for debugging help!

### Cache Issues

If you suspect cache corruption:

```bash
# Clear specific cache
docker volume rm kodi-depends-cache

# Next run will rebuild only that component
./scripts/build-local-android.sh
```

### Disk Space

```bash
# Check Docker disk usage
docker system df

# Clean up old images/containers (keeps volumes)
docker system prune

# Clean up everything including volumes
docker system prune -a --volumes
```

## Advanced Usage

### Manual Docker Commands

If you want to debug interactively:

```bash
# Start an interactive container with the caches mounted
docker run -it --rm \
  -v $(pwd):/workspace \
  -v kodi-source-cache:/opt/kodi \
  -v kodi-depends-cache:/opt/xbmc-depends \
  -v android-sdk-cache:/opt/android-sdk \
  pvr-jellyfin-android-builder:latest \
  bash

# Inside the container, you can run commands manually
cd /workspace
# ... run build commands ...
```

### Updating Dependencies

If Kodi updates or dependencies change:

```bash
# Remove the dependency cache to rebuild
docker volume rm kodi-depends-cache

# The next build will rebuild dependencies with latest code
./scripts/build-local-android.sh
```

### Building Specific Versions

```bash
# Checkout specific version first
git checkout v1.0.0

# Run build (will use that version)
./scripts/build-local-android.sh

# Switch back
git checkout main
```

## Integration with Development Workflow

### Typical Development Cycle

1. **Make code changes** in your editor
2. **Commit changes** to git
3. **Run local build** to test:
   ```bash
   ./scripts/build-local-android.sh
   ```
4. **Test the generated addon** on Android device
5. **Create release** (already done by the script!)
6. **Push to GitHub** (if build succeeded):
   ```bash
   git push origin main
   ```

### Testing Without Release

If you want to build without creating a release, you can comment out the release creation step in the script or run individual steps manually.

## CI/CD Integration

This local build script complements the GitHub Actions workflow:

- **Local**: Fast iteration with cached dependencies
- **CI**: Automated builds on every push

Both use the same Docker image and build process, ensuring consistency!

## FAQ

**Q: How long does the first build take?**
A: 30-60 minutes. Subsequent builds: 5-10 minutes.

**Q: Can I build for other platforms?**
A: This script is specifically for Android ARM32. For other platforms, use the standard Kodi build system.

**Q: What if the script updates itself?**
A: The script runs `git pull` before building, so it always uses the latest version from GitHub.

**Q: Can I run this on Windows?**
A: Yes, using WSL2 (Windows Subsystem for Linux) with Docker Desktop.

**Q: How do I update Kodi version?**
A: Edit `Dockerfile.android-build` and change `KODI_VERSION=Omega` to the desired branch.

## Getting Help

- **Build fails**: The script creates a gist automatically - share that URL
- **Docker issues**: Check Docker documentation or run `docker system info`
- **GitHub CLI issues**: Run `gh auth status` to check authentication
- **General questions**: Open an issue on GitHub

## License

This script is part of the pvr.jellyfin project and uses the same license (see LICENSE file).
