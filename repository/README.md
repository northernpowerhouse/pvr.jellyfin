# Jellyfin PVR Addon Repository

This directory contains the Kodi addon repository structure for easy installation and updates of the Jellyfin PVR client.

## Installation

1. Download `repository.jellyfin.pvr.zip` from the releases
2. In Kodi, go to Add-ons > Install from zip file
3. Select the downloaded zip file
4. Once installed, you can install the Jellyfin PVR Client from:
   Add-ons > Install from repository > Jellyfin PVR Repository > PVR clients

## Structure

```
repository/
├── repository.jellyfin.pvr/    # Repository addon
│   └── addon.xml
├── pvr.jellyfin/               # PVR addon zips
│   └── [version].zip
├── addons.xml                  # Generated addon list
└── addons.xml.md5             # Checksum for addons.xml
```

## Automatic Updates

The repository is automatically updated via GitHub Actions when a new version is tagged and released.
