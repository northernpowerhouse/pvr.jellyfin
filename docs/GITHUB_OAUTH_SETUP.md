# GitHub OAuth App Configuration

## For Log Upload Feature

The log upload feature uses GitHub Device Flow authentication to allow users to upload logs to the repository without hardcoding credentials.

### OAuth App Details:
- **App Name**: pvr.jellyfin Log Uploader
- **Client ID**: `Ov23liXOKSJZpGo5qJqF`
- **Authorization URL**: https://github.com/login/device
- **Token URL**: https://github.com/login/oauth/access_token
- **Scopes Required**: `repo` (for writing to the repository)

### How It Works:
1. User clicks "Upload Logs to GitHub" in addon settings
2. Addon initiates device flow and displays a code
3. User goes to github.com/login/device and enters the code
4. Once authorized, the addon receives a token
5. Token is used to upload log files to `dev-logs/` folder

### For Repository Owner:
The OAuth app should be created in the repository owner's GitHub account:
1. Go to https://github.com/settings/developers
2. Click "OAuth Apps" → "New OAuth App"
3. Configure:
   - Application name: `pvr.jellyfin Log Uploader`
   - Homepage URL: `https://github.com/northernpowerhouse/pvr.jellyfin`
   - Authorization callback URL: `https://github.com/northernpowerhouse/pvr.jellyfin` (not used for device flow)
4. After creation, note the Client ID
5. Update `GITHUB_CLIENT_ID` in `src/utilities/LogUploader.cpp`

### Security Notes:
- Device flow is used instead of hardcoded credentials
- Users authenticate with their own GitHub accounts
- Tokens are stored only in memory during the session
- Each user needs repository write access to upload logs
