# Installation Guide

This guide provides detailed instructions for installing the Jellyfin PVR Client on Kodi.

## Prerequisites

### Kodi Requirements
- Kodi 21 "Omega" or later
- Live TV & PVR enabled in Kodi settings

### Jellyfin Requirements
- Jellyfin Server 10.9.x or later
- Live TV configured with at least one tuner source
- User account with Live TV permissions
- API key generated for the addon

## Jellyfin Server Setup

### 1. Configure Live TV

1. Log in to your Jellyfin dashboard
2. Go to **Dashboard > Live TV**
3. Add a TV source (tuner device, IPTV, etc.)
4. Configure EPG provider if desired
5. Ensure channels are visible in the Live TV section

### 2. Generate API Key

1. Go to **Dashboard > API Keys**
2. Click the **"+"** button
3. Enter a name: `Kodi PVR Client`
4. Copy the generated API key - you'll need this for addon configuration

### 3. Find Your User ID

1. Go to **Dashboard > Users**
2. Click on your username
3. Look at the URL - it will contain your user ID
4. Example: `http://server:8096/web/index.html#!/users/user.html?userId=abc123def456`
5. The user ID is `abc123def456`

## Installation Methods

### Method 1: From Repository (Recommended)

This method allows for automatic updates.

1. Download the repository addon:
   - Go to the [Releases](https://github.com/northernpowerhouse/pvr.jellyfin/releases) page
   - Download `repository.jellyfin.pvr.zip`

2. Install the repository:
   - In Kodi, go to **Settings > Add-ons**
   - Click **Install from zip file**
   - Browse to the downloaded `repository.jellyfin.pvr.zip`
   - Wait for the "Addon installed" notification

3. Install the PVR client:
   - Go to **Install from repository**
   - Select **Jellyfin PVR Repository**
   - Navigate to **PVR clients**
   - Select **Jellyfin PVR Client**
   - Click **Install**
   - Wait for installation to complete

### Method 2: Direct Installation

Install directly from the addon zip file.

1. Download the addon:
   - Go to the [Releases](https://github.com/northernpowerhouse/pvr.jellyfin/releases) page
   - Download the appropriate `pvr.jellyfin-*.zip` for your platform

2. Install the addon:
   - In Kodi, go to **Settings > Add-ons**
   - Click **Install from zip file**
   - Browse to the downloaded addon zip
   - Wait for the "Addon installed" notification

### Method 3: From Source

For developers or those who want to build from source.

1. Clone the repository:
   ```bash
   git clone https://github.com/northernpowerhouse/pvr.jellyfin.git
   cd pvr.jellyfin
   ```

2. Build the addon:
   ```bash
   ./scripts/build.sh
   ```

3. Package the addon:
   ```bash
   ./scripts/package.sh 1.0.0
   ```

4. Install the generated zip file using Method 2

## Configuration

### 1. Enable the Addon

1. Go to **Settings > Add-ons > My add-ons > PVR clients**
2. Select **Jellyfin PVR Client**
3. Click **Enable** if not already enabled

### 2. Configure Settings

1. With the addon selected, click **Configure**
2. Enter your Jellyfin server details:

   **Connection Settings:**
   - **Server URL**: Full URL to your Jellyfin server
     - Example: `http://192.168.1.100:8096`
     - Include `http://` or `https://`
     - Do not include trailing slash
   
   - **User ID**: Your Jellyfin user ID (from setup step)
     - Example: `abc123def456`
   
   - **API Key**: The API key you generated
     - Example: `1234567890abcdef1234567890abcdef`

   **EPG Settings:**
   - **Enable EPG**: Keep enabled for program guide
   - **EPG Update Interval**: How often to refresh EPG (default: 120 minutes)

3. Click **OK** to save settings

### 3. Enable Live TV in Kodi

1. Go to **Settings > PVR & Live TV > General**
2. Ensure **Enabled** is checked
3. The addon should now appear in the list of enabled PVR clients

### 4. Verify Connection

1. Go to **TV > Channels**
2. Channels should appear within a few seconds
3. If no channels appear, check the Kodi log for errors

## Troubleshooting

### No Channels Appear

1. **Check Jellyfin Live TV:**
   - Verify Live TV is working in Jellyfin web interface
   - Ensure at least one channel is available

2. **Verify Settings:**
   - Double-check server URL, user ID, and API key
   - Ensure no extra spaces in settings

3. **Check Network:**
   - Verify Kodi can reach Jellyfin server
   - Test URL in a web browser from the Kodi device
   - Check firewall settings

4. **Check Logs:**
   - Go to **Settings > System > Logging**
   - Enable debug logging
   - Restart Kodi
   - Check `kodi.log` for errors related to `pvr.jellyfin`

### Streaming Issues

1. **Test in Jellyfin Web:**
   - Verify streaming works in Jellyfin web interface
   - This rules out tuner/source issues

2. **Check Transcoding:**
   - Jellyfin may need to transcode the stream
   - Ensure Jellyfin has transcoding capability
   - Check Jellyfin server performance

3. **Network Bandwidth:**
   - Live TV requires consistent bandwidth
   - Test network speed between Kodi and Jellyfin
   - Consider quality/bitrate settings in Jellyfin

### EPG Not Showing

1. **Wait for Initial Load:**
   - EPG data may take a few minutes to load initially

2. **Check EPG in Jellyfin:**
   - Verify EPG data is available in Jellyfin
   - Check EPG provider configuration

3. **Adjust Update Interval:**
   - Try reducing EPG update interval in settings

### Recording Issues

1. **Check Jellyfin Recording Path:**
   - Verify recording path is configured in Jellyfin
   - Ensure Jellyfin has write permissions

2. **Check Disk Space:**
   - Ensure sufficient disk space for recordings

3. **Verify Timer Creation:**
   - Check Jellyfin dashboard to see if timer was created
   - Timers should appear in Jellyfin's Live TV section

## Getting Help

If you continue to experience issues:

1. **Check Documentation:**
   - Read the [README](../README.md)
   - Review [Contributing Guide](../CONTRIBUTING.md)

2. **Check Logs:**
   - Enable Kodi debug logging
   - Check Jellyfin server logs
   - Look for error messages

3. **Report Issues:**
   - Open an issue on [GitHub](https://github.com/northernpowerhouse/pvr.jellyfin/issues)
   - Include Kodi version, Jellyfin version, and platform
   - Provide relevant log excerpts
   - Describe steps to reproduce

## Uninstallation

To remove the addon:

1. Go to **Settings > Add-ons > My add-ons > PVR clients**
2. Select **Jellyfin PVR Client**
3. Click **Uninstall**
4. Confirm the uninstallation

To also remove the repository:

1. Go to **Settings > Add-ons > My add-ons > Add-on repository**
2. Select **Jellyfin PVR Repository**
3. Click **Uninstall**
