# Development Guide

This guide helps developers set up their environment and contribute to the Jellyfin PVR addon.

## Development Environment Setup

### Prerequisites

#### Required Tools
- **C++ Compiler**: GCC 7+, Clang 5+, or MSVC 2017+
- **CMake**: 3.5 or later
- **Git**: For version control
- **Python 3**: For build scripts

#### Platform-Specific Requirements

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
sudo apt-get install libjsoncpp-dev
sudo apt-get install kodi-pvr-dev  # If available in repos
```

**macOS:**
```bash
brew install cmake jsoncpp
# Install Xcode Command Line Tools
xcode-select --install
```

**Windows:**
```powershell
# Install Visual Studio 2019/2022 with C++ workload
# Install CMake from cmake.org
# Install vcpkg for dependencies
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install jsoncpp:x64-windows
```

### Kodi Development Headers

The addon requires Kodi development headers. You can:

1. **Build Kodi from source** (recommended for development)
2. **Install kodi-dev packages** (if available for your distro)
3. **Download headers** from Kodi repository

#### Building Kodi from Source (Linux)
```bash
git clone --depth=1 https://github.com/xbmc/xbmc.git kodi
cd kodi
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
```

## Project Setup

### Clone Repository
```bash
git clone https://github.com/northernpowerhouse/pvr.jellyfin.git
cd pvr.jellyfin
```

### Configure Build
```bash
mkdir build
cd build

# Basic build
cmake ..

# Or specify Kodi path
cmake .. -DKODI_INCLUDE_DIR=/path/to/kodi/include

# For release build
cmake .. -DCMAKE_BUILD_TYPE=Release

# For debug build with symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### Build Project
```bash
# From build directory
cmake --build .

# Or use make
make

# For parallel build (faster)
make -j$(nproc)
```

### Install for Testing
```bash
# Install to Kodi addons directory
sudo make install

# Or copy manually
cp pvr.jellyfin.so ~/.kodi/addons/pvr.jellyfin/
```

## Code Structure

### Directory Layout
```
pvr.jellyfin/
├── src/
│   ├── client.cpp/h              # Main addon entry point
│   ├── jellyfin/                 # Jellyfin API integration
│   │   ├── JellyfinClient.cpp/h  # Main client class
│   │   ├── Connection.cpp/h      # HTTP/JSON handling
│   │   ├── ChannelManager.cpp/h  # Channel operations
│   │   ├── EPGManager.cpp/h      # EPG operations
│   │   └── RecordingManager.cpp/h # Recording/timer operations
│   └── utilities/                # Helper classes
│       ├── Logger.cpp/h          # Logging wrapper
│       └── Utilities.cpp/h       # String/date utilities
└── resources/                    # Addon resources
    ├── settings.xml              # User settings
    └── language/                 # Translations
```

### Key Classes

#### CJellyfinPVRClient (client.cpp/h)
- Main PVR interface implementation
- Inherits from `kodi::addon::CInstancePVRClient`
- Handles all PVR API callbacks

#### JellyfinClient (jellyfin/JellyfinClient.cpp/h)
- High-level Jellyfin integration
- Manages connection and sub-managers
- Provides unified interface to Jellyfin features

#### Connection (jellyfin/Connection.cpp/h)
- HTTP request/response handling
- JSON parsing with JsonCpp
- Authentication header management

#### Managers
- **ChannelManager**: Channel and channel group operations
- **EPGManager**: EPG data retrieval and parsing
- **RecordingManager**: Recording and timer operations

## Development Workflow

### 1. Make Changes
Edit source files in `src/` directory.

### 2. Build
```bash
cd build
make
```

### 3. Install/Test
```bash
# Install to test in Kodi
sudo make install

# Or for quick testing, copy directly
cp pvr.jellyfin.so ~/.kodi/addons/pvr.jellyfin/
```

### 4. Debug
```bash
# Start Kodi with debug logging
kodi --debug

# Or view logs in real-time
tail -f ~/.kodi/temp/kodi.log | grep pvr.jellyfin
```

### 5. Iterate
Repeat steps 1-4 until feature is complete.

## Coding Standards

### Style Guide
- **Indentation**: 2 spaces (no tabs)
- **Line Length**: 100 characters max
- **Braces**: Same line for functions, control statements
- **Naming**:
  - Classes: `PascalCase`
  - Functions: `PascalCase`
  - Variables: `camelCase`
  - Member variables: `m_camelCase`
  - Constants: `UPPER_SNAKE_CASE`

### Example
```cpp
class MyClass
{
public:
  MyClass() = default;
  void DoSomething(const std::string& input);

private:
  std::string m_memberVariable;
  static const int MAX_RETRIES = 3;
};

void MyClass::DoSomething(const std::string& input)
{
  if (input.empty())
  {
    Logger::Log(ADDON_LOG_ERROR, "Input is empty");
    return;
  }
  
  m_memberVariable = input;
}
```

### Best Practices
- Use RAII for resource management
- Prefer smart pointers over raw pointers
- Use const correctness
- Handle all error cases
- Log important events and errors
- Add comments for complex logic
- Keep functions focused and small

## Adding Features

### Adding a New PVR Function

1. **Add to client.h:**
```cpp
PVR_ERROR GetNewFeature(kodi::addon::PVRNewType& result);
```

2. **Implement in client.cpp:**
```cpp
PVR_ERROR CJellyfinPVRClient::GetNewFeature(kodi::addon::PVRNewType& result)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;
    
  return m_jellyfinClient->GetNewFeature(result);
}
```

3. **Add to JellyfinClient.h:**
```cpp
PVR_ERROR GetNewFeature(kodi::addon::PVRNewType& result);
```

4. **Implement in JellyfinClient.cpp:**
```cpp
PVR_ERROR JellyfinClient::GetNewFeature(kodi::addon::PVRNewType& result)
{
  // Implementation or delegate to manager
  if (m_someManager)
    return m_someManager->GetNewFeature(result);
  return PVR_ERROR_SERVER_ERROR;
}
```

5. **Add to appropriate Manager:**
Implement the actual Jellyfin API call and data processing.

### Adding a New Setting

1. **Add to resources/settings.xml:**
```xml
<setting id="new_setting" type="text" label="30020" default="value">
  <level>0</level>
</setting>
```

2. **Add translation in resources/language/*/strings.po:**
```
msgctxt "#30020"
msgid "New Setting"
msgstr ""
```

3. **Read in client.cpp:**
```cpp
m_newSetting = kodi::addon::GetSettingString("new_setting", "default");
```

## Debugging

### Debug Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
```

### GDB Debugging
```bash
# Start Kodi in GDB
gdb --args kodi

# Set breakpoint
(gdb) break JellyfinClient::Connect
(gdb) run

# When breakpoint hits
(gdb) print variableName
(gdb) step
(gdb) continue
```

### Visual Studio Code
Create `.vscode/launch.json`:
```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug Kodi",
      "type": "cppdbg",
      "request": "launch",
      "program": "/usr/bin/kodi",
      "args": ["--debug"],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb"
    }
  ]
}
```

### Logging
```cpp
// Use Logger class for all logging
Logger::Log(ADDON_LOG_INFO, "Info message");
Logger::Log(ADDON_LOG_ERROR, "Error: %s", errorMsg.c_str());
Logger::Log(ADDON_LOG_DEBUG, "Debug value: %d", value);
```

### Common Issues

**Issue**: Addon doesn't load in Kodi
- Check kodi.log for errors
- Verify addon.xml is valid
- Ensure library file exists

**Issue**: Compilation errors
- Update Kodi headers to match version
- Check all includes are available
- Verify JsonCpp is installed

**Issue**: Crashes on startup
- Check for null pointer dereferences
- Verify all resources are initialized
- Use debug build with GDB

## Testing

### Unit Tests
Currently, the project doesn't have formal unit tests. Consider adding:
- Google Test framework
- Mock Jellyfin responses
- Test individual managers

### Integration Tests
- Test against real Jellyfin server
- Verify all PVR functions
- Check error handling

### Manual Testing
Follow the [Testing Guide](TESTING.md) for comprehensive manual testing.

## Pull Request Process

1. **Fork** the repository
2. **Create** a feature branch
3. **Make** changes following coding standards
4. **Test** thoroughly
5. **Commit** with clear messages
6. **Push** to your fork
7. **Create** pull request with description

### PR Checklist
- [ ] Code follows style guide
- [ ] All tests pass
- [ ] New features have tests
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
- [ ] No compiler warnings
- [ ] Tested on target platform

## Release Process

1. **Update version** in addon.xml.in
2. **Update CHANGELOG.md** with changes
3. **Create tag**: `git tag -a v1.0.0 -m "Release 1.0.0"`
4. **Push tag**: `git push origin v1.0.0`
5. **GitHub Actions** builds and creates release
6. **Verify** release artifacts

## Resources

### Documentation
- [Kodi PVR API](https://github.com/xbmc/xbmc/blob/master/xbmc/addons/kodi-dev-kit/include/kodi/addon-instance/PVR.h)
- [Jellyfin API](https://api.jellyfin.org/)
- [Kodi Addon Development](https://kodi.wiki/view/Add-on_development)

### Example Addons
- [pvr.iptvsimple](https://github.com/kodi-pvr/pvr.iptvsimple)
- [pvr.hts](https://github.com/kodi-pvr/pvr.hts)
- [pvr.demo](https://github.com/kodi-pvr/pvr.demo)

### Community
- [Kodi Forum](https://forum.kodi.tv/forumdisplay.php?fid=26)
- [Jellyfin Forum](https://forum.jellyfin.org/)
- [GitHub Issues](https://github.com/northernpowerhouse/pvr.jellyfin/issues)

## Tips and Tricks

- **Incremental builds**: CMake caches configuration, just run `make` after first build
- **Clean build**: `rm -rf build && mkdir build && cd build && cmake .. && make`
- **Quick test**: Change code → `make` → restart Kodi
- **Log filtering**: `grep pvr.jellyfin kodi.log` to see only addon logs
- **JSON validation**: Use `jq` to validate API responses
- **Memory debugging**: Use Valgrind to detect leaks

## Getting Help

- **Documentation**: Check docs/ directory
- **Issues**: Search existing GitHub issues
- **Forum**: Post in Kodi forum
- **Chat**: Join discussion on GitHub

Happy developing!
