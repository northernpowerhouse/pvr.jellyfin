# Frequently Asked Questions (FAQ)

## General Questions

### Q: What is this addon?
**A:** This is a PVR (Personal Video Recorder) client addon for Kodi that integrates with Jellyfin's Live TV functionality. It allows you to watch live TV, browse the EPG (Electronic Program Guide), and manage recordings through Kodi using your Jellyfin server as the backend.

### Q: What's the difference between this and the regular Jellyfin addon?
**A:** The regular Jellyfin addon for Kodi provides access to your media library (movies, TV shows, music). This PVR addon specifically provides Live TV and DVR functionality through Kodi's PVR interface, giving you a native TV viewing experience in Kodi.

### Q: Can I use both addons together?
**A:** Yes! The Jellyfin media addon and this PVR addon can coexist. Use the media addon for your recorded content library and this PVR addon for live TV and DVR features.

## Requirements

### Q: What versions of Kodi and Jellyfin do I need?
**A:** 
- Kodi 21 "Omega" or later
- Jellyfin Server 10.9.x or later

### Q: Do I need a TV tuner?
**A:** Yes, your Jellyfin server needs to be configured with at least one Live TV source (TV tuner, IPTV, etc.). The addon itself doesn't require hardware on the Kodi device.

### Q: Does this work on all platforms?
**A:** The addon is designed to work on all platforms that support Kodi 21+, including:
- Linux (x86_64, ARM)
- Windows (x86_64)
- macOS (x86_64, Apple Silicon)
- Android (ARM, x86)
- iOS (with sideloading)
- LibreELEC, CoreELEC

## Setup and Configuration

### Q: How do I get my Jellyfin API key?
**A:** 
1. Log in to Jellyfin dashboard
2. Go to Dashboard > API Keys
3. Click the "+" button
4. Enter a name (e.g., "Kodi PVR")
5. Copy the generated key

### Q: How do I find my Jellyfin User ID?
**A:** 
1. Go to Dashboard > Users
2. Click your username
3. The URL will contain your user ID
4. Example: `...userId=abc123def456` → your ID is `abc123def456`

### Q: What format should my server URL be?
**A:** Use the complete URL including protocol:
- Good: `http://192.168.1.100:8096`
- Good: `https://jellyfin.mydomain.com`
- Bad: `192.168.1.100` (missing protocol)
- Bad: `http://jellyfin.local:8096/` (has trailing slash)

### Q: Can I use HTTPS with a self-signed certificate?
**A:** Currently, this depends on your Kodi/OS trust store. You may need to add your self-signed certificate to the system trust store.

## Features

### Q: Does this support radio channels?
**A:** The addon is designed primarily for TV channels. Radio channel support is not currently enabled but could be added in future versions.

### Q: Can I schedule series recordings?
**A:** Series recording support is planned for a future release. Currently, you can schedule individual recordings.

### Q: Does it support recording groups/folders?
**A:** Recordings are organized by series name as provided by Jellyfin.

### Q: Can I watch recordings while they're still recording?
**A:** This depends on Jellyfin's capabilities. Generally, you should be able to watch in-progress recordings with a slight delay.

## Troubleshooting

### Q: Why don't any channels appear?
**A:**
1. Verify Live TV works in Jellyfin web interface
2. Check server URL, user ID, and API key are correct
3. Ensure your user has permission to access Live TV
4. Check Kodi log for error messages
5. Try restarting the addon

### Q: Why is the EPG empty?
**A:**
1. Verify EPG is working in Jellyfin
2. Wait a few minutes for initial load
3. Check EPG update interval in settings
4. Ensure Jellyfin has an EPG provider configured
5. Check Kodi debug log for EPG-related errors

### Q: Live TV won't play - it just buffers
**A:**
1. Test the stream in Jellyfin web interface
2. Check network speed between Kodi and Jellyfin
3. Verify Jellyfin can transcode if needed
4. Check firewall isn't blocking streaming
5. Try reducing quality settings in Jellyfin

### Q: The addon says "PVR Manager is disabled"
**A:**
1. Go to Settings > PVR & Live TV > General
2. Ensure "Enabled" is checked
3. Restart Kodi

### Q: Recordings won't delete
**A:**
1. Check Jellyfin permissions for your user
2. Verify recording still exists in Jellyfin
3. Check Jellyfin server logs for errors
4. Try deleting from Jellyfin web interface

### Q: Timers aren't creating in Jellyfin
**A:**
1. Verify recording path is set in Jellyfin
2. Check disk space on Jellyfin server
3. Ensure user has recording permissions
4. Check Jellyfin logs for error messages

## Performance

### Q: Why is the addon slow to load?
**A:**
- Initial load includes channel list and EPG data
- Large channel counts take longer
- Network speed affects load time
- EPG for many channels can be slow

### Q: How much bandwidth does live TV use?
**A:** This depends on:
- Source stream bitrate
- Whether Jellyfin transcodes
- Network conditions
- Typically 2-10 Mbps for HD content

### Q: Does this work over the internet?
**A:** Yes, but:
- You need sufficient upload speed from Jellyfin server
- Jellyfin should be accessible externally (port forwarding or VPN)
- Consider using HTTPS for security
- May need transcoding for bandwidth limitations

## Compatibility

### Q: Does this work with HDHomeRun?
**A:** Yes, if your Jellyfin server is configured with HDHomeRun as a Live TV source.

### Q: Does this work with IPTV?
**A:** Yes, if your Jellyfin server has IPTV configured as a Live TV source.

### Q: Can I use multiple Jellyfin servers?
**A:** Currently, only one Jellyfin server is supported per addon instance. You would need to configure separate profiles or Kodi instances for multiple servers.

### Q: Does this work with Emby?
**A:** No, this addon is specifically for Jellyfin. However, since Jellyfin is based on Emby, the API is similar, and an Emby version could potentially be created.

## Comparison with Other Solutions

### Q: Should I use this or IPTV Simple Client?
**A:** Use this addon if:
- You have a Jellyfin server with Live TV
- You want integrated recording management
- You want direct Jellyfin integration

Use IPTV Simple Client if:
- You only need basic IPTV playback
- You don't use Jellyfin
- You have M3U playlists

### Q: How is this different from TVHeadend?
**A:** TVHeadend is a complete TV server solution, while this addon uses Jellyfin as the backend. Choose based on your existing setup:
- Use this if you already use Jellyfin
- Use TVHeadend if you want a dedicated TV server

## Development and Contributing

### Q: Is this open source?
**A:** Yes, the project is licensed under CC0 1.0 Universal and hosted on GitHub.

### Q: Can I contribute?
**A:** Absolutely! See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

### Q: How do I report a bug?
**A:**
1. Check if it's already reported on GitHub
2. Enable Kodi debug logging
3. Reproduce the issue
4. Create a GitHub issue with:
   - Kodi version and platform
   - Jellyfin version
   - Steps to reproduce
   - Log excerpts

### Q: How do I request a feature?
**A:** Open a GitHub issue with the "enhancement" label, describing:
- What you want to achieve
- Why it would be useful
- Any implementation suggestions

## Updates

### Q: How do I update the addon?
**A:** If installed from repository:
1. Kodi will notify you of updates
2. Go to Add-ons > My add-ons > PVR clients > Jellyfin PVR Client
3. Select "Update" if available

If installed from zip:
1. Download new version
2. Install from zip (will upgrade)

### Q: Will updating delete my settings?
**A:** No, your settings are preserved across updates.

### Q: How often is the addon updated?
**A:** Updates are released as needed for bug fixes and new features. Watch the GitHub repository for announcements.

## Privacy and Security

### Q: Is my API key secure?
**A:** The API key is stored in Kodi's settings and should be treated as a password. It's not transmitted except to your configured Jellyfin server.

### Q: What data does the addon collect?
**A:** The addon doesn't collect or transmit any data except to your specified Jellyfin server. All communication is between Kodi and your Jellyfin server.

### Q: Should I use HTTPS?
**A:** Yes, especially if accessing Jellyfin over the internet. HTTPS encrypts communication between Kodi and Jellyfin.

## Advanced

### Q: Can I customize the addon behavior?
**A:** Limited customization is available through settings. For more advanced changes, you can modify the source code.

### Q: Does this support DVB subtitles?
**A:** Subtitle support depends on Kodi's video player and Jellyfin's transcoding capabilities.

### Q: Can I use hardware transcoding?
**A:** Transcoding is handled by Jellyfin server. Configure hardware transcoding in Jellyfin's dashboard.

### Q: Does this work with Kodi's PVR recording features?
**A:** Yes, the addon integrates with Kodi's PVR recording interface, but the actual recording is done by Jellyfin server.

## Still Need Help?

If your question isn't answered here:

1. Check the [Installation Guide](INSTALLATION.md)
2. Read the [README](../README.md)
3. Search [GitHub Issues](https://github.com/northernpowerhouse/pvr.jellyfin/issues)
4. Post on the [Kodi Forum](https://forum.kodi.tv/)
5. Open a new [GitHub Issue](https://github.com/northernpowerhouse/pvr.jellyfin/issues/new)

Remember to include:
- Kodi version and platform
- Jellyfin version
- Description of your issue
- Steps to reproduce
- Relevant log excerpts
