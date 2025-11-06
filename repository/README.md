# Jellyfin PVR Addon Repository

This directory contains the Kodi addon repository that enables automatic updates of the pvr.jellyfin addon.

## How It Works

1. **Users install the repository addon** (`repository.jellyfin.pvr-*.zip`) in Kodi
2. **Kodi checks for updates** by reading `addons.xml` from this GitHub repository
3. **Both addons can auto-update**:
   - `pvr.jellyfin` - The main PVR addon (downloads from GitHub Releases)
   - `repository.jellyfin.pvr` - The repository itself (self-updating!)

## Installation

1. Download `repository.jellyfin.pvr-*.zip` from [releases](https://github.com/northernpowerhouse/pvr.jellyfin/releases)
2. In Kodi, go to **Add-ons → Install from zip file**
3. Select the downloaded repository zip file
4. Once installed, you can install pvr.jellyfin from:
   **Add-ons → Install from repository → Jellyfin PVR Repository → PVR clients**

## Structure

```
repository/
├── repository.jellyfin.pvr/    # Repository addon source
│   └── addon.xml              # Points to this repo's addons.xml
├── addons.xml                  # Lists both addons (generated)
├── addons.xml.md5             # Checksum for verification
└── README.md                   # This file
```

## Update Process

When a new version is released:

1. Build script generates `addons.xml` with current versions
2. `addons.xml` lists **both** addons so the repository can self-update
3. **Commit and push** `addons.xml` and `addons.xml.md5` to main branch
4. Kodi fetches these files from raw.githubusercontent.com
5. Users get automatic update notifications in Kodi

## Important URLs

Kodi reads addon information from:
- `https://raw.githubusercontent.com/northernpowerhouse/pvr.jellyfin/main/repository/addons.xml`
- `https://raw.githubusercontent.com/northernpowerhouse/pvr.jellyfin/main/repository/addons.xml.md5`

Addon ZIP files are downloaded from GitHub Releases:
- `https://github.com/northernpowerhouse/pvr.jellyfin/releases/download/{version}/pvr.jellyfin-{version}.zip`
- `https://github.com/northernpowerhouse/pvr.jellyfin/releases/download/{version}/repository.jellyfin.pvr-{version}.zip`

## Manual Regeneration

To manually regenerate `addons.xml`:

```bash
./scripts/update_addons_xml.sh "v1.0.0"
git add repository/addons.xml repository/addons.xml.md5
git commit -m "Update repository metadata for v1.0.0"
git push
```
