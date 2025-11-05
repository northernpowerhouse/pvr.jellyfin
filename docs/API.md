# Jellyfin API Integration

This document describes how the addon integrates with the Jellyfin API.

## Authentication

The addon uses API key authentication with the following header:
```
X-Emby-Authorization: MediaBrowser Client="Kodi", Device="Kodi", DeviceId="kodi-pvr-jellyfin", Version="1.0.0"
```

API key is passed as a query parameter: `?api_key=YOUR_API_KEY`

## API Endpoints Used

### System Information
**Endpoint:** `GET /System/Info`

**Purpose:** Verify connection and get server version

**Response:**
```json
{
  "Version": "10.9.0",
  "OperatingSystem": "Linux",
  ...
}
```

### Live TV Channels
**Endpoint:** `GET /LiveTv/Channels?userId={userId}`

**Purpose:** Retrieve list of available TV channels

**Response:**
```json
{
  "Items": [
    {
      "Id": "channel-id",
      "Name": "Channel Name",
      "ChannelNumber": 1,
      "Type": "TvChannel",
      "ImageTags": {
        "Primary": "..."
      }
    }
  ],
  "TotalRecordCount": 10
}
```

### Channel Groups
**Endpoint:** `GET /LiveTv/ChannelGroups?userId={userId}`

**Purpose:** Retrieve channel groups/categories

**Response:**
```json
{
  "Items": [
    {
      "Id": "group-id",
      "Name": "Group Name"
    }
  ]
}
```

### EPG Programs
**Endpoint:** `GET /LiveTv/Programs?userId={userId}&minStartDate={date}&maxStartDate={date}`

**Purpose:** Retrieve Electronic Program Guide data

**Parameters:**
- `minStartDate`: ISO 8601 start date (e.g., "2024-01-01T00:00:00Z")
- `maxStartDate`: ISO 8601 end date
- `channelId` (optional): Filter by specific channel

**Response:**
```json
{
  "Items": [
    {
      "Id": "program-id",
      "ChannelId": "channel-id",
      "Name": "Program Title",
      "Overview": "Program description",
      "StartDate": "2024-01-01T20:00:00Z",
      "EndDate": "2024-01-01T21:00:00Z",
      "EpisodeTitle": "Episode Name",
      "IndexNumber": 1,
      "ParentalRating": "TV-14"
    }
  ]
}
```

### Recordings
**Endpoint:** `GET /LiveTv/Recordings?userId={userId}`

**Purpose:** Retrieve list of recorded programs

**Response:**
```json
{
  "Items": [
    {
      "Id": "recording-id",
      "Name": "Recording Title",
      "SeriesName": "Series Name",
      "ChannelName": "Channel Name",
      "Overview": "Description",
      "StartDate": "2024-01-01T20:00:00Z",
      "EndDate": "2024-01-01T21:00:00Z",
      "UserData": {
        "PlayCount": 0
      }
    }
  ]
}
```

### Delete Recording
**Endpoint:** `DELETE /LiveTv/Recordings/{recordingId}`

**Purpose:** Delete a recorded program

### Timers (Scheduled Recordings)
**Endpoint:** `GET /LiveTv/Timers?userId={userId}`

**Purpose:** Retrieve scheduled recordings

**Response:**
```json
{
  "Items": [
    {
      "Id": "timer-id",
      "Name": "Timer Name",
      "ChannelId": "channel-id",
      "StartDate": "2024-01-01T20:00:00Z",
      "EndDate": "2024-01-01T21:00:00Z",
      "Status": "New"
    }
  ]
}
```

### Create Timer
**Endpoint:** `POST /LiveTv/Timers`

**Purpose:** Schedule a new recording

**Request Body:**
```json
{
  "Name": "Timer Name",
  "ChannelId": "channel-id",
  "StartDate": "2024-01-01T20:00:00Z",
  "EndDate": "2024-01-01T21:00:00Z"
}
```

### Delete Timer
**Endpoint:** `DELETE /LiveTv/Timers/{timerId}`

**Purpose:** Cancel a scheduled recording

### Live Stream URL
**Format:** `{serverUrl}/LiveTv/LiveStreamFiles/{channelId}/stream.m3u8?api_key={apiKey}`

**Purpose:** Get HLS stream URL for live TV channel

### Recording Stream URL
**Format:** `{serverUrl}/Videos/{recordingId}/stream?static=true&api_key={apiKey}`

**Purpose:** Get stream URL for recorded content

## Data Mapping

### Channel UID Generation
Kodi requires integer UIDs for channels. The addon generates these by:
1. Hashing the Jellyfin channel ID string
2. Taking the lower 31 bits (to ensure positive integer)

```cpp
std::hash<std::string> hasher;
int uid = static_cast<int>(hasher(channel.id) & 0x7FFFFFFF);
```

### Date/Time Format
Jellyfin uses ISO 8601 format: `YYYY-MM-DDTHH:MM:SSZ`

The addon uses `strftime` and `strptime` for conversion.

### Channel Types
- `TvChannel` - TV channel (radio = false)
- `RadioChannel` - Radio channel (radio = true)

## Error Handling

The addon handles API errors by:
1. Checking HTTP response success
2. Validating JSON parsing
3. Logging errors with details
4. Returning appropriate PVR error codes

Common PVR error codes:
- `PVR_ERROR_NO_ERROR` - Success
- `PVR_ERROR_SERVER_ERROR` - Connection/API failure
- `PVR_ERROR_INVALID_PARAMETERS` - Invalid data
- `PVR_ERROR_FAILED` - General failure

## Rate Limiting

The addon implements these rate limiting strategies:
1. EPG data is cached and refreshed at configurable intervals (default: 120 minutes)
2. Channel list is loaded once on startup
3. Recordings and timers are loaded on-demand

## Future Enhancements

Potential API integrations:
1. `/LiveTv/Programs/Recommended` - Recommended programs
2. `/LiveTv/SeriesTimers` - Series recording support
3. `/Users/{userId}/Items` - Additional metadata
4. `/Sessions/Playing` - Playback status updates
5. WebSocket connections for real-time updates

## References

- [Jellyfin API Documentation](https://api.jellyfin.org/)
- [Jellyfin OpenAPI Spec](https://api.jellyfin.org/openapi/api-docs.json)
- [Emby API Reference](https://github.com/MediaBrowser/Emby/wiki) (Jellyfin is based on Emby)
