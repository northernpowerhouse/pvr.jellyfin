# Jellyfin PVR Client for Kodi

A Kodi PVR addon that integrates Jellyfin's Live TV functionality into Kodi, allowing you to watch live TV, browse the EPG, and manage recordings from your Jellyfin server.

## Features

- **Live TV Channels**: Browse and watch live TV channels from your Jellyfin server
- **Electronic Program Guide (EPG)**: View program information and schedules
- **Channel Groups**: Organize channels into groups
- **Recordings**: View and manage your recorded programs
- **Timers**: Schedule recordings for future programs
- **Streaming**: Direct streaming from Jellyfin server with HLS support

## Requirements

- Kodi 21 "Omega" or later
- Jellyfin Server 10.9.x or later with Live TV configured
- Jellyfin API key for authentication

## Installation

### From Repository (Recommended)

1. Download the addon repository zip file
2. In Kodi, go to Add-ons > Install from zip file
3. Select the repository zip file
4. Go to Add-ons > Install from repository > Jellyfin PVR Repository
5. Select PVR clients > Jellyfin PVR Client > Install

### Manual Installation

1. Download the latest release zip file
2. In Kodi, go to Add-ons > Install from zip file
3. Select the addon zip file
4. Enable the addon in Settings > PVR & Live TV > General

## Configuration

1. Navigate to Add-ons > My add-ons > PVR clients > Jellyfin PVR Client
2. Click Configure
3. Enter your Jellyfin server settings:
   - **Server URL**: Your Jellyfin server URL (e.g., `http://192.168.1.100:8096`)
   - **User ID**: Your Jellyfin user ID (found in Jellyfin dashboard)
   - **API Key**: Your Jellyfin API key (generated in Jellyfin dashboard under API Keys)
4. Save the settings
5. The addon will connect to your Jellyfin server and load channels

### Getting Your API Key

1. Log in to your Jellyfin server dashboard
2. Go to Dashboard > API Keys
3. Click the "+" button to create a new API key
4. Give it a name (e.g., "Kodi PVR")
5. Copy the generated API key and paste it into the addon settings

## Building from Source

### Prerequisites

- CMake 3.5 or later
- C++ compiler with C++17 support
- Kodi development headers
- JsonCpp library

### Build Instructions

```bash
git clone https://github.com/northernpowerhouse/pvr.jellyfin.git
cd pvr.jellyfin
mkdir build
cd build
cmake ..
make
```

## Troubleshooting

### Addon won't connect to Jellyfin

- Verify your server URL is correct and accessible
- Check that your API key is valid
- Ensure Jellyfin Live TV is configured with at least one tuner
- Check Kodi logs for error messages

### No channels appear

- Verify that channels are set up in Jellyfin Live TV
- Check that the user has permission to access Live TV
- Try restarting the addon

### Streaming issues

- Ensure your network connection is stable
- Check that the Jellyfin server can transcode if needed
- Verify firewall settings allow streaming

## Development

### Project Structure

```
pvr.jellyfin/
├── src/
│   ├── client.cpp/h           # Main addon entry point
│   ├── jellyfin/
│   │   ├── JellyfinClient.cpp/h    # Main Jellyfin client
│   │   ├── Connection.cpp/h         # HTTP connection handler
│   │   ├── ChannelManager.cpp/h     # Channel management
│   │   ├── EPGManager.cpp/h         # EPG data management
│   │   └── RecordingManager.cpp/h   # Recording management
│   └── utilities/
│       ├── Logger.cpp/h             # Logging utilities
│       └── Utilities.cpp/h          # Helper functions
├── resources/
│   ├── settings.xml            # Addon settings
│   └── language/              # Translations
├── addon.xml.in               # Addon metadata template
└── CMakeLists.txt            # Build configuration
```

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## License

This addon is licensed under CC0 1.0 Universal. See the LICENSE file for details.

## Acknowledgments

- Kodi PVR API documentation
- Jellyfin API documentation
- The Kodi and Jellyfin communities