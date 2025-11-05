# Testing Guide

This document provides guidance on testing the Jellyfin PVR addon.

## Test Environment Setup

### Required Components
1. **Jellyfin Server** (10.9.x or later)
   - Configure with Live TV tuner
   - Set up EPG provider
   - Create test user with API key

2. **Kodi Installation** (21 "Omega" or later)
   - Fresh install recommended for testing
   - Enable debug logging
   - Enable Live TV & PVR

3. **Test Content**
   - At least 5 TV channels
   - EPG data for channels
   - At least 1 recording
   - At least 1 scheduled timer

## Testing Checklist

### 1. Installation Testing

- [ ] Addon installs successfully from zip
- [ ] Repository installs correctly
- [ ] Addon appears in PVR clients list
- [ ] Settings can be opened and saved
- [ ] Addon enables without errors

### 2. Connection Testing

- [ ] Valid credentials connect successfully
- [ ] Invalid URL shows appropriate error
- [ ] Invalid API key shows appropriate error
- [ ] Server version is detected correctly
- [ ] Connection survives network interruption

### 3. Channel Testing

- [ ] Channels load on first connection
- [ ] Channel count matches Jellyfin
- [ ] Channel names display correctly
- [ ] Channel numbers are correct
- [ ] Channel logos appear
- [ ] Channels appear in Kodi TV section

### 4. Channel Groups Testing

- [ ] Channel groups load correctly
- [ ] Group names display properly
- [ ] Group membership is accurate
- [ ] Switching between groups works
- [ ] Empty groups don't cause errors

### 5. EPG Testing

- [ ] EPG data loads for channels
- [ ] Current program displays correctly
- [ ] Future programs show in guide
- [ ] EPG time range is appropriate
- [ ] Program descriptions are complete
- [ ] Series/episode info displays
- [ ] EPG refreshes at configured interval

### 6. Live TV Streaming

- [ ] Channel playback starts successfully
- [ ] Video quality is acceptable
- [ ] Audio is synchronized
- [ ] Channel switching works smoothly
- [ ] Pause/resume functions properly
- [ ] Stream recovers from buffering
- [ ] Different channels test different codecs

### 7. Recording Testing

- [ ] Recordings list loads
- [ ] Recording metadata is accurate
- [ ] Thumbnails display (if available)
- [ ] Recording playback works
- [ ] Recording deletion works
- [ ] Deleted recordings disappear
- [ ] Recording count updates correctly

### 8. Timer Testing

- [ ] Timer list loads correctly
- [ ] Can schedule new recording
- [ ] Timer appears in Jellyfin
- [ ] Can delete timer
- [ ] Timer deletion syncs to Jellyfin
- [ ] Timer conflicts are handled
- [ ] Series timers (if supported)

### 9. Performance Testing

- [ ] Startup time is acceptable
- [ ] Channel list loads within 5 seconds
- [ ] EPG loads within 10 seconds
- [ ] Memory usage is stable
- [ ] No memory leaks over 24 hours
- [ ] CPU usage is reasonable
- [ ] Network traffic is appropriate

### 10. Error Handling

- [ ] Graceful handling of server offline
- [ ] Appropriate error for missing channels
- [ ] Network timeout handled correctly
- [ ] Invalid stream URL fails gracefully
- [ ] API errors are logged clearly
- [ ] Recovery after errors works

### 11. Settings Testing

- [ ] All settings save correctly
- [ ] Settings persist after restart
- [ ] Setting changes take effect
- [ ] Invalid settings are rejected
- [ ] Default settings are reasonable

### 12. Multi-Platform Testing

- [ ] Works on Linux (x86_64)
- [ ] Works on Windows (x86_64)
- [ ] Works on macOS (x86_64/ARM)
- [ ] Works on Android (ARM)
- [ ] Works on Raspberry Pi
- [ ] Works on LibreELEC

## Automated Testing

### Build Tests
```bash
# Test build process
./scripts/build.sh

# Verify build artifacts
ls build/

# Check for compilation warnings
grep -i warning build.log
```

### Package Tests
```bash
# Test packaging
./scripts/package.sh 1.0.0

# Verify zip structure
unzip -l pvr.jellyfin-1.0.0.zip

# Check file permissions
unzip -l pvr.jellyfin-1.0.0.zip | grep -E "^-rwx"
```

## Manual Testing Scenarios

### Scenario 1: First Time Setup
1. Install addon from zip
2. Configure settings
3. Verify connection
4. Browse channels
5. Watch live TV
6. Check EPG
7. Test recording playback

### Scenario 2: Daily Use
1. Open Kodi Live TV
2. Browse EPG for tonight
3. Schedule recording
4. Watch live channel
5. Switch channels
6. Pause and resume
7. Exit and verify recording scheduled

### Scenario 3: Recording Management
1. View recordings list
2. Play a recording
3. Watch for 2 minutes
4. Stop playback
5. Delete recording
6. Verify deletion in Jellyfin

### Scenario 4: Error Recovery
1. Start watching live TV
2. Disconnect network
3. Wait 30 seconds
4. Reconnect network
5. Verify stream recovers
6. Check for error messages

### Scenario 5: Settings Change
1. Change server URL
2. Save settings
3. Restart addon
4. Verify new connection
5. Revert settings
6. Verify channels restored

## Log Analysis

### Enable Kodi Debug Logging
1. Settings > System > Logging
2. Enable "Enable debug logging"
3. Restart Kodi

### Check Logs
```bash
# Linux
tail -f ~/.kodi/temp/kodi.log | grep pvr.jellyfin

# Windows
type %APPDATA%\Kodi\kodi.log | findstr pvr.jellyfin

# macOS
tail -f ~/Library/Logs/kodi.log | grep pvr.jellyfin
```

### Look For
- Connection attempts
- API call failures
- Stream errors
- Memory issues
- Timing problems

## Performance Benchmarks

### Expected Performance
- Addon startup: < 2 seconds
- Channel list load: < 5 seconds
- EPG initial load: < 10 seconds
- Stream start: < 3 seconds
- Channel switch: < 2 seconds
- Memory usage: < 50 MB
- CPU usage (idle): < 1%

### Measuring Performance
```bash
# Memory usage
ps aux | grep kodi

# CPU usage
top -p $(pidof kodi)

# Network traffic
nethogs
```

## Known Issues Testing

Test against known issues documented in CHANGELOG.md:
1. Channel UID mapping edge cases
2. EPG channel filtering accuracy
3. Timer channel mapping in some scenarios

## Regression Testing

When making changes, verify:
1. All previous tests still pass
2. No new warnings in logs
3. Performance hasn't degraded
4. Settings still work
5. All features still functional

## Reporting Test Results

When reporting test results, include:
- Kodi version and platform
- Jellyfin server version
- Test checklist results
- Log excerpts for failures
- Screenshots of UI issues
- Steps to reproduce problems

## Continuous Integration

The GitHub Actions workflow tests:
1. Build on multiple platforms
2. Code compilation
3. Dependency resolution
4. Package creation
5. Repository generation

CI runs automatically on:
- Every push to main
- Every pull request
- Every tag creation

## Test Data Setup

### Creating Test Jellyfin Server
```bash
# Using Docker
docker run -d \
  --name jellyfin-test \
  -p 8096:8096 \
  -v /path/to/config:/config \
  -v /path/to/media:/media \
  jellyfin/jellyfin:latest
```

### Adding Test Channels
1. Use IPTV source with public m3u
2. Or use HDHomeRun/tuner device
3. Add EPG from XMLTV source
4. Create test recordings

## Security Testing

- [ ] API key not logged in plain text
- [ ] Sensitive data not in screenshots
- [ ] HTTPS works correctly
- [ ] Invalid certificates handled
- [ ] SQL injection not possible
- [ ] XSS not possible

## Accessibility Testing

- [ ] Screen reader compatible
- [ ] Keyboard navigation works
- [ ] High contrast mode supported
- [ ] Text scaling works
- [ ] All features accessible

## Conclusion

Thorough testing ensures the addon works reliably for all users. Report any issues found during testing on GitHub with detailed information to help with debugging and fixes.
