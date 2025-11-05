# Contributing to Jellyfin PVR Client

Thank you for considering contributing to the Jellyfin PVR Client! This document provides guidelines and information for contributors.

## Getting Started

### Prerequisites

- Kodi 21 "Omega" or later development environment
- C++ compiler with C++17 support
- CMake 3.5 or later
- JsonCpp library
- Git

### Setting Up Development Environment

1. Clone the repository:
   ```bash
   git clone https://github.com/northernpowerhouse/pvr.jellyfin.git
   cd pvr.jellyfin
   ```

2. Install dependencies (Ubuntu/Debian):
   ```bash
   sudo apt-get install cmake build-essential libjsoncpp-dev
   ```

3. Build the addon:
   ```bash
   ./scripts/build.sh
   ```

## Development Workflow

### Making Changes

1. Create a new branch for your feature or bugfix:
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. Make your changes, following the coding standards below

3. Test your changes thoroughly

4. Commit your changes with clear, descriptive commit messages:
   ```bash
   git commit -m "Add feature: description of what you did"
   ```

5. Push to your fork and create a pull request

### Coding Standards

- Follow the existing code style
- Use meaningful variable and function names
- Add comments for complex logic
- Keep functions focused and single-purpose
- Use C++17 features appropriately
- Handle errors gracefully with proper logging

### Testing

- Test with a real Jellyfin server
- Verify all PVR functions work correctly:
  - Channel listing
  - EPG display
  - Live TV playback
  - Recording management
  - Timer functionality
- Check for memory leaks
- Test on multiple platforms if possible

## Project Structure

```
pvr.jellyfin/
├── src/
│   ├── client.cpp/h              # Main addon interface
│   ├── jellyfin/
│   │   ├── JellyfinClient.cpp/h  # Core client logic
│   │   ├── Connection.cpp/h      # HTTP communication
│   │   ├── ChannelManager.cpp/h  # Channel handling
│   │   ├── EPGManager.cpp/h      # EPG data
│   │   └── RecordingManager.cpp/h # Recordings/timers
│   └── utilities/
│       ├── Logger.cpp/h          # Logging
│       └── Utilities.cpp/h       # Helper functions
├── resources/
│   ├── settings.xml              # User settings
│   └── language/                 # Translations
└── scripts/                      # Build/packaging scripts
```

## Adding Features

### Adding a New Jellyfin API Endpoint

1. Add the API call in `Connection.cpp/h`
2. Implement the logic in the appropriate manager class
3. Expose the functionality through `JellyfinClient.cpp/h`
4. Add the PVR interface in `client.cpp/h`

### Adding Settings

1. Add the setting in `resources/settings.xml`
2. Add translations in `resources/language/*/strings.po`
3. Read the setting in `client.cpp` using `kodi::addon::GetSettingString()` or similar
4. Use the setting in your code

## Jellyfin API Reference

Key Jellyfin API endpoints used:

- `/System/Info` - Server information
- `/LiveTv/Channels` - Channel list
- `/LiveTv/Programs` - EPG data
- `/LiveTv/Recordings` - Recorded programs
- `/LiveTv/Timers` - Scheduled recordings
- `/LiveTv/LiveStreamFiles/{id}/stream.m3u8` - Live stream URL

Full API documentation: https://api.jellyfin.org/

## Kodi PVR API Reference

Key PVR API methods implemented:

- `GetChannels()` - Provide channel list
- `GetEPGForChannel()` - Provide EPG data
- `GetRecordings()` - Provide recordings
- `GetTimers()` - Provide scheduled recordings
- `GetChannelStreamProperties()` - Provide stream URL

Full PVR API documentation: https://github.com/xbmc/xbmc/blob/master/xbmc/addons/kodi-dev-kit/include/kodi/addon-instance/PVR.h

## Submitting Pull Requests

1. Ensure your code follows the coding standards
2. Update documentation if needed
3. Add a clear description of your changes
4. Reference any related issues
5. Ensure CI checks pass

## Reporting Issues

When reporting issues, please include:

- Kodi version
- Jellyfin server version
- Operating system
- Steps to reproduce
- Expected behavior
- Actual behavior
- Relevant log excerpts from Kodi

## License

By contributing, you agree that your contributions will be licensed under the CC0 1.0 Universal License.

## Questions?

Feel free to open an issue for questions or discussions!
