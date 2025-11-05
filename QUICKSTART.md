# Quick Start Guide

Get up and running with the Jellyfin PVR addon in 5 minutes!

## Prerequisites Check

Before you start, make sure you have:

- [ ] Kodi 21 "Omega" or later installed
- [ ] Jellyfin Server 10.9.x or later running
- [ ] Live TV configured in Jellyfin (tuner or IPTV)
- [ ] Jellyfin API key generated
- [ ] Your Jellyfin user ID

## Step 1: Get Your Jellyfin Credentials (5 minutes)

### Generate API Key

1. Open Jellyfin web interface
2. Click **Dashboard** (hamburger menu top-left)
3. Navigate to **API Keys**
4. Click **+** button
5. Enter name: `Kodi PVR`
6. Copy the generated key (you'll need this!)

### Find Your User ID

1. In Jellyfin dashboard, go to **Users**
2. Click your username
3. Look at the browser URL bar
4. Find the ID after `userId=`
5. Example: `http://jellyfin:8096/web/index.html#!/users/user.html?userId=abc123`
6. Your ID is: `abc123`

## Step 2: Install the Addon (2 minutes)

### Option A: From Repository (Recommended)

1. Download `repository.jellyfin.pvr.zip` from [Releases](https://github.com/northernpowerhouse/pvr.jellyfin/releases)
2. In Kodi: **Settings** → **Add-ons** → **Install from zip file**
3. Select the repository zip
4. Wait for notification: "Add-on installed"
5. Go to **Install from repository** → **Jellyfin PVR Repository**
6. Navigate to **PVR clients** → **Jellyfin PVR Client**
7. Click **Install**

### Option B: Direct Install

1. Download `pvr.jellyfin-*.zip` from [Releases](https://github.com/northernpowerhouse/pvr.jellyfin/releases)
2. In Kodi: **Settings** → **Add-ons** → **Install from zip file**
3. Select the addon zip
4. Wait for notification

## Step 3: Configure the Addon (2 minutes)

1. Go to **Settings** → **Add-ons** → **My add-ons** → **PVR clients**
2. Select **Jellyfin PVR Client**
3. Click **Configure**
4. Enter your settings:
   - **Server URL**: `http://your-jellyfin-ip:8096` (no trailing slash!)
   - **User ID**: Your ID from Step 1
   - **API Key**: Your key from Step 1
5. Click **OK** to save

## Step 4: Enable Live TV (1 minute)

1. Go to **Settings** → **PVR & Live TV** → **General**
2. Ensure **Enabled** is checked
3. The addon should appear in the enabled clients list

## Step 5: Watch TV! (Now!)

1. Go to **TV** → **Channels** from Kodi main menu
2. Your channels should appear within a few seconds
3. Select a channel and enjoy!

## Quick Troubleshooting

### No channels appear?

```bash
# Check these in order:
1. Is Live TV working in Jellyfin web interface?
2. Did you enter the server URL correctly? (include http://)
3. Is the API key correct? (no extra spaces)
4. Can Kodi reach your Jellyfin server? (try pinging)
5. Check Kodi logs: Settings → System → Logging → Enable debug
```

### Can't connect to server?

```bash
# Verify your URL:
Good: http://192.168.1.100:8096
Bad:  192.168.1.100:8096           (missing http://)
Bad:  http://192.168.1.100:8096/   (has trailing slash)
Bad:  http://jellyfin                (use IP or full hostname)
```

### EPG not showing?

Wait 2-3 minutes for initial load. EPG data can take time to download from Jellyfin.

## Next Steps

Now that you're set up:

- **Browse EPG**: Press **Guide** button or use **TV** → **Guide**
- **Schedule Recordings**: Select a future program and choose "Record"
- **View Recordings**: Go to **TV** → **Recordings**
- **Organize Channels**: Use channel groups from **TV** → **Channels** → **Groups**

## Getting Help

Still stuck?

1. **Read the FAQ**: [docs/FAQ.md](docs/FAQ.md)
2. **Check Installation Guide**: [docs/INSTALLATION.md](docs/INSTALLATION.md)
3. **Enable Debug Logging**: Settings → System → Logging
4. **Check Logs**: Look for `pvr.jellyfin` errors
5. **Ask for Help**: [GitHub Issues](https://github.com/northernpowerhouse/pvr.jellyfin/issues)

When asking for help, include:
- Kodi version and platform
- Jellyfin version
- Error messages from log
- What you tried

## Pro Tips

### Better Performance
- Use wired network instead of WiFi
- Enable hardware transcoding in Jellyfin if needed
- Keep Jellyfin and Kodi up to date

### Better Experience
- Configure EPG update interval (Settings → Configure)
- Set up channel logos in Jellyfin
- Organize channels into groups
- Use series recording for your favorite shows (coming soon!)

### Security
- Use HTTPS if accessing over internet
- Keep your API key private
- Use a strong Jellyfin password
- Enable two-factor auth in Jellyfin

## Success!

You should now be watching Live TV through Kodi with your Jellyfin server as the backend. Enjoy!

For advanced features and customization, see the [full documentation](docs/).

---

**Need more help?** Check the [FAQ](docs/FAQ.md) or [open an issue](https://github.com/northernpowerhouse/pvr.jellyfin/issues)!
