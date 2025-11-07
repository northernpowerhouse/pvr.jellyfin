# Live TV Playback Fix - DeviceProfile Structure

## Problem
Live TV playback was failing with HTTP 400 error from the `/Items/{id}/PlaybackInfo` endpoint. The Jellyfin server was rejecting our DeviceProfile structure.

## Investigation Process

### 1. Initial Debugging (Commits 1-6)
- Tried multiple API endpoints:
  - `POST /LiveTv/LiveStreams/Open` → 404 (wrong endpoint)
  - `POST /Items/{id}/PlaybackInfo` → 400 (DeviceProfile validation failure)
- Added debug logging to capture exact JSON being sent

### 2. Comparative Analysis (Commit 7+)
- User installed official jellyfin-kodi addon alongside our PVR addon
- Analyzed log comparison between working (official) and broken (ours) addons
- Official addon successfully received MediaSources with LiveStreamId
- Examined jellyfin-kodi source code on GitHub

### 3. Root Cause Identified
Our DeviceProfile had incorrect DirectPlayProfiles structure:

**WRONG (Our Version):**
```json
"DirectPlayProfiles": [
  {
    "Type": "Video",
    "Container": "",
    "VideoCodec": "h264,hevc,mpeg4,mpeg2video,vc1,vp9,av1",
    "AudioCodec": "aac,mp3,ac3,eac3,flac,alac,vorbis,opus"
  }
]
```

**CORRECT (Official jellyfin-kodi):**
```json
"DirectPlayProfiles": [
  {
    "Type": "Video",
    "VideoCodec": "h264,hevc,mpeg4,mpeg2video,vc1,vp9,av1"
  }
]
```

## The Fix
**Commit 8d72f48**: Removed `Container` and `AudioCodec` fields from Video DirectPlayProfiles

The Jellyfin server's DeviceProfile validation requires:
- Video DirectPlayProfiles: `Type` + `VideoCodec` only
- Audio DirectPlayProfiles: `Type` only  
- Photo DirectPlayProfiles: `Type` only
- Also added back `TimelineOffsetSeconds: 5` to match official addon

## DeviceProfile Structure Reference
Based on jellyfin-kodi addon (`jellyfin_kodi/helper/playutils.py` lines 420-515):

```cpp
Json::Value deviceProfile;
deviceProfile["Name"] = "Kodi";
deviceProfile["MaxStreamingBitrate"] = 120000000;
deviceProfile["MaxStaticBitrate"] = 120000000;
deviceProfile["MusicStreamingTranscodingBitrate"] = 1280000;
deviceProfile["TimelineOffsetSeconds"] = 5;

// DirectPlayProfiles - Simple structure
Json::Value directPlayProfiles(Json::arrayValue);
videoDirectPlay["Type"] = "Video";
videoDirectPlay["VideoCodec"] = "h264,hevc,mpeg4,mpeg2video,vc1,vp9,av1";
// NO Container or AudioCodec fields!

audioDirectPlay["Type"] = "Audio";
// NO codec fields for Audio

photoDirectPlay["Type"] = "Photo";
// NO fields for Photo

// TranscodingProfiles - More detailed
Json::Value transcodingProfiles(Json::arrayValue);
// Contains Container, AudioCodec, VideoCodec, etc.

// Empty arrays
deviceProfile["ResponseProfiles"] = Json::Value(Json::arrayValue);
deviceProfile["ContainerProfiles"] = Json::Value(Json::arrayValue);
deviceProfile["CodecProfiles"] = Json::Value(Json::arrayValue);
deviceProfile["SubtitleProfiles"] = Json::Value(Json::arrayValue);
```

## Expected Outcome
With the corrected DeviceProfile:
1. `POST /Items/{channelId}/PlaybackInfo` should return HTTP 200
2. Response contains `MediaSources` array with `LiveStreamId` and `MediaSourceId`
3. Use these IDs to construct stream URL: `/videos/{channelId}/live.m3u8?LiveStreamId={id}&MediaSourceId={id}&api_key={key}`

## Testing
User needs to:
1. Build updated addon (commit 8d72f48)
2. Attempt to play a live TV channel
3. Check logs for:
   - Successful PlaybackInfo request (no HTTP 400)
   - MediaSources received with LiveStreamId
   - Stream URL construction and playback

## Lessons Learned
1. **Debug logging is critical** - Logging exact JSON requests revealed the issue
2. **Compare with working implementations** - Official addon showed correct structure
3. **Server validation is strict** - Extra fields cause rejection, not just missing fields
4. **Field names matter** - Container/AudioCodec in DirectPlayProfiles was the problem
5. **Documentation gaps** - Jellyfin API docs don't clearly show DeviceProfile validation rules

## References
- Jellyfin API: https://api.jellyfin.org/
- Official jellyfin-kodi addon: https://github.com/jellyfin/jellyfin-kodi
- Key file: `jellyfin_kodi/helper/playutils.py` (get_device_profile method)
- Jellyfin server: 10.11.1
- Kodi version: Omega v21.2
