#!/bin/bash

# Jellyfin API Test Script
# Tests various API endpoints to find the correct way to get live TV streams

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Load configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_FILE="${SCRIPT_DIR}/test_jellyfin_api.config"

if [ ! -f "$CONFIG_FILE" ]; then
    echo -e "${RED}Error: Configuration file not found: $CONFIG_FILE${NC}"
    echo "Please create the config file with your settings."
    exit 1
fi

source "$CONFIG_FILE"

# Create output directory (relative to script location)
OUTPUT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)/devlog/apitest"
mkdir -p "$OUTPUT_DIR"
TIMESTAMP=$(date +%Y%m%d%H%M)
LOG_FILE="${OUTPUT_DIR}/${TIMESTAMP}.log"

# Logging function
log() {
    echo -e "$1" | tee -a "$LOG_FILE"
}

log_section() {
    log "\n${BLUE}============================================================${NC}"
    log "${BLUE}$1${NC}"
    log "${BLUE}============================================================${NC}\n"
}

log_success() {
    log "${GREEN}✓ $1${NC}"
}

log_error() {
    log "${RED}✗ $1${NC}"
}

log_info() {
    log "${YELLOW}ℹ $1${NC}"
}

# Save JSON response
save_response() {
    local name="$1"
    local response="$2"
    local file="${OUTPUT_DIR}/${TIMESTAMP}_${name}.json"
    echo "$response" | jq '.' > "$file" 2>/dev/null || echo "$response" > "$file"
    log_info "Response saved to: $file"
}

# Make API request
api_request() {
    local method="$1"
    local endpoint="$2"
    local data="$3"
    local auth_header="$4"
    local extra_headers="$5"
    
    local url="${SERVER_URL}${endpoint}"
    local curl_opts=(-s -w "\nHTTP_STATUS:%{http_code}" -X "$method")
    
    if [ -n "$auth_header" ]; then
        curl_opts+=(-H "$auth_header")
    fi
    
    if [ -n "$extra_headers" ]; then
        IFS='|' read -ra HEADERS <<< "$extra_headers"
        for header in "${HEADERS[@]}"; do
            curl_opts+=(-H "$header")
        done
    fi
    
    if [ -n "$data" ]; then
        curl_opts+=(-H "Content-Type: application/json" -d "$data")
    fi
    
    curl_opts+=("$url")
    
    local response=$(curl "${curl_opts[@]}")
    local http_code=$(echo "$response" | grep "HTTP_STATUS:" | cut -d: -f2)
    local body=$(echo "$response" | sed '/HTTP_STATUS:/d')
    
    echo "$http_code|$body"
}

# Build Device Profile variations
get_device_profile_v1() {
    cat <<'EOF'
{
  "MaxStreamingBitrate": 120000000,
  "MaxStaticBitrate": 100000000,
  "MusicStreamingTranscodingBitrate": 384000,
  "DirectPlayProfiles": [
    {"Type": "Video", "VideoCodec": "h264,hevc,mpeg2video,vc1"},
    {"Type": "Audio"},
    {"Type": "Photo"}
  ],
  "TranscodingProfiles": [
    {
      "Type": "Video",
      "Container": "ts",
      "VideoCodec": "h264",
      "AudioCodec": "aac,mp3,ac3",
      "Protocol": "hls",
      "Context": "Streaming"
    },
    {
      "Type": "Video",
      "Container": "mp4",
      "VideoCodec": "h264",
      "AudioCodec": "aac,mp3,ac3",
      "Protocol": "http",
      "Context": "Streaming"
    }
  ],
  "ContainerProfiles": [],
  "CodecProfiles": [],
  "SubtitleProfiles": []
}
EOF
}

get_device_profile_v2() {
    cat <<'EOF'
{
  "MaxStreamingBitrate": 120000000,
  "DirectPlayProfiles": [
    {"Type": "Video"}
  ],
  "TranscodingProfiles": [
    {
      "Type": "Video",
      "Container": "ts",
      "VideoCodec": "h264",
      "AudioCodec": "aac",
      "Protocol": "hls"
    }
  ]
}
EOF
}

get_device_profile_minimal() {
    cat <<'EOF'
{
  "DirectPlayProfiles": [
    {"Type": "Video"}
  ]
}
EOF
}

get_device_profile_passthrough() {
    cat <<'EOF'
{
  "Name": "Kodi",
  "MaxStreamingBitrate": 120000000,
  "MaxStaticBitrate": 120000000,
  "DirectPlayProfiles": [
    {"Type": "Video"},
    {"Type": "Audio"}
  ],
  "TranscodingProfiles": [],
  "ContainerProfiles": [],
  "CodecProfiles": [],
  "SubtitleProfiles": []
}
EOF
}

# Start testing
log_section "Jellyfin Live TV API Test - $TIMESTAMP"
log "Server: $SERVER_URL"

# Parse channel IDs
IFS=',' read -ra CHANNEL_ARRAY <<< "$CHANNEL_IDS"
log "Testing ${#CHANNEL_ARRAY[@]} channel(s): ${CHANNEL_IDS}"

# Test 1: Authenticate with username/password
log_section "TEST 1: Authenticate with Username/Password"
AUTH_BODY=$(cat <<EOF
{
  "Username": "$USERNAME",
  "Pw": "$PASSWORD"
}
EOF
)

result=$(api_request "POST" "/Users/AuthenticateByName" "$AUTH_BODY" "" "X-Emby-Authorization: MediaBrowser Client=\"$CLIENT_NAME\", Device=\"$DEVICE_NAME\", DeviceId=\"$DEVICE_ID\", Version=\"$CLIENT_VERSION\"")
http_code=$(echo "$result" | cut -d'|' -f1)
response=$(echo "$result" | cut -d'|' -f2-)

if [ "$http_code" = "200" ]; then
    log_success "Authentication successful (HTTP $http_code)"
    AUTH_TOKEN=$(echo "$response" | jq -r '.AccessToken')
    CURRENT_USER_ID=$(echo "$response" | jq -r '.User.Id')
    log_info "Access Token: ${AUTH_TOKEN:0:20}..."
    log_info "User ID: $CURRENT_USER_ID"
    save_response "auth_response" "$response"
else
    log_error "Authentication failed (HTTP $http_code)"
    save_response "auth_error" "$response"
fi

# Set up authorization headers
X_EMBY_AUTH="X-Emby-Authorization: MediaBrowser Client=\"$CLIENT_NAME\", Device=\"$DEVICE_NAME\", DeviceId=\"$DEVICE_ID\", Version=\"$CLIENT_VERSION\", Token=\"$AUTH_TOKEN\""
X_EMBY_AUTH_API="X-Emby-Authorization: MediaBrowser Client=\"$CLIENT_NAME\", Device=\"$DEVICE_NAME\", DeviceId=\"$DEVICE_ID\", Version=\"$CLIENT_VERSION\", Token=\"$API_KEY\""

# Test 2: Get channel info
log_section "TEST 2: GET /Items/{ChannelId} - Get Channel Info"

# Just test the first channel with both auth methods
CHANNEL_ID="${CHANNEL_ARRAY[0]}"
log_info "Testing first channel: $CHANNEL_ID"

for auth_type in "TOKEN" "APIKEY"; do
    log_info "Testing with $auth_type"
    if [ "$auth_type" = "TOKEN" ]; then
        auth_header="$X_EMBY_AUTH"
    else
        auth_header="$X_EMBY_AUTH_API"
    fi
    
    result=$(api_request "GET" "/Items/$CHANNEL_ID" "" "$auth_header")
    http_code=$(echo "$result" | cut -d'|' -f1)
    response=$(echo "$result" | cut -d'|' -f2-)
    
    if [ "$http_code" = "200" ]; then
        log_success "Channel info retrieved (HTTP $http_code) with $auth_type"
        save_response "channel_info_${auth_type}" "$response"
        
        # Check for MediaSources
        media_sources=$(echo "$response" | jq -r '.MediaSources // empty')
        if [ -n "$media_sources" ] && [ "$media_sources" != "null" ]; then
            log_success "MediaSources found in channel info!"
            echo "$response" | jq '.MediaSources' | tee -a "$LOG_FILE"
        else
            log_info "No MediaSources in channel info"
        fi
    else
        log_error "Failed to get channel info (HTTP $http_code) with $auth_type"
        save_response "channel_info_error_${auth_type}" "$response"
    fi
done

# Test 3: PlaybackInfo with different DeviceProfiles
log_section "TEST 3: POST /Items/{ChannelId}/PlaybackInfo - Standard Method"

for profile_type in "v1" "v2" "minimal" "passthrough" "none"; do
    log_info "Testing with DeviceProfile: $profile_type"
    
    case $profile_type in
        "v1")
            device_profile=$(get_device_profile_v1)
            ;;
        "v2")
            device_profile=$(get_device_profile_v2)
            ;;
        "minimal")
            device_profile=$(get_device_profile_minimal)
            ;;
        "passthrough")
            device_profile=$(get_device_profile_passthrough)
            ;;
        "none")
            device_profile="{}"
            ;;
    esac
    
    playback_body=$(cat <<EOF
{
  "UserId": "$CURRENT_USER_ID",
  "DeviceProfile": $device_profile
}
EOF
)
    
    result=$(api_request "POST" "/Items/$CHANNEL_ID/PlaybackInfo" "$playback_body" "$X_EMBY_AUTH")
    http_code=$(echo "$result" | cut -d'|' -f1)
    response=$(echo "$result" | cut -d'|' -f2-)
    
    if [ "$http_code" = "200" ]; then
        log_success "PlaybackInfo successful (HTTP $http_code) with $profile_type"
        save_response "playbackinfo_${profile_type}" "$response"
        
        # Extract stream URLs
        media_sources=$(echo "$response" | jq -r '.MediaSources // empty')
        if [ -n "$media_sources" ] && [ "$media_sources" != "null" ]; then
            log_success "MediaSources found!"
            echo "$response" | jq '.MediaSources[] | {Path: .Path, Id: .Id, Protocol: .Protocol}' | tee -a "$LOG_FILE"
        fi
    else
        log_error "PlaybackInfo failed (HTTP $http_code) with $profile_type"
        save_response "playbackinfo_error_${profile_type}" "$response"
    fi
done

# Test 4: LiveTV specific endpoints
log_section "TEST 4: GET /LiveTv/Channels/{ChannelId} - LiveTV Endpoint"
result=$(api_request "GET" "/LiveTv/Channels/$CHANNEL_ID" "" "$X_EMBY_AUTH")
http_code=$(echo "$result" | cut -d'|' -f1)
response=$(echo "$result" | cut -d'|' -f2-)

if [ "$http_code" = "200" ]; then
    log_success "LiveTV Channel info retrieved (HTTP $http_code)"
    save_response "livetv_channel" "$response"
else
    log_error "Failed to get LiveTV channel (HTTP $http_code)"
    save_response "livetv_channel_error" "$response"
fi

# Test 5: Try to get MediaSourceInfo
log_section "TEST 5: GET /LiveTv/LiveStreamMediaInfo - Media Source Info"
result=$(api_request "GET" "/LiveTv/LiveStreamMediaInfo?ChannelId=$CHANNEL_ID" "" "$X_EMBY_AUTH")
http_code=$(echo "$result" | cut -d'|' -f1)
response=$(echo "$result" | cut -d'|' -f2-)

if [ "$http_code" = "200" ]; then
    log_success "LiveStreamMediaInfo retrieved (HTTP $http_code)"
    save_response "livestream_mediainfo" "$response"
else
    log_error "LiveStreamMediaInfo failed (HTTP $http_code)"
    save_response "livestream_mediainfo_error" "$response"
fi

# Test 6: Open live stream
log_section "TEST 6: POST /LiveTv/LiveStreams/Open - Open Live Stream"
for device_profile_type in "v1" "minimal" "none"; do
    log_info "Testing with DeviceProfile: $device_profile_type"
    
    case $device_profile_type in
        "v1")
            device_profile=$(get_device_profile_v1)
            ;;
        "minimal")
            device_profile=$(get_device_profile_minimal)
            ;;
        "none")
            device_profile="{}"
            ;;
    esac
    
    open_body=$(cat <<EOF
{
  "UserId": "$CURRENT_USER_ID",
  "ItemId": "$CHANNEL_ID",
  "DeviceProfile": $device_profile
}
EOF
)
    
    result=$(api_request "POST" "/LiveTv/LiveStreams/Open" "$open_body" "$X_EMBY_AUTH")
    http_code=$(echo "$result" | cut -d'|' -f1)
    response=$(echo "$result" | cut -d'|' -f2-)
    
    if [ "$http_code" = "200" ]; then
        log_success "Live stream opened (HTTP $http_code) with $device_profile_type"
        save_response "livestream_open_${device_profile_type}" "$response"
        
        # Extract stream info
        stream_path=$(echo "$response" | jq -r '.MediaSource.Path // empty')
        stream_id=$(echo "$response" | jq -r '.MediaSource.Id // empty')
        
        if [ -n "$stream_path" ]; then
            log_success "Stream Path: $stream_path"
            log_info "Stream ID: $stream_id"
            
            # Test 7: Close the stream
            log_info "Closing stream..."
            result=$(api_request "POST" "/LiveTv/LiveStreams/Close" "{\"LiveStreamId\": \"$stream_id\"}" "$X_EMBY_AUTH")
            http_code=$(echo "$result" | cut -d'|' -f1)
            if [ "$http_code" = "204" ] || [ "$http_code" = "200" ]; then
                log_success "Stream closed (HTTP $http_code)"
            else
                log_error "Failed to close stream (HTTP $http_code)"
            fi
        fi
    else
        log_error "Failed to open live stream (HTTP $http_code) with $device_profile_type"
        save_response "livestream_open_error_${device_profile_type}" "$response"
    fi
done

# Test 6b: Try opening with OpenToken from PlaybackInfo
log_section "TEST 6b: POST /LiveTv/LiveStreams/Open with OpenToken"
log_info "First, get PlaybackInfo with AutoOpenLiveStream=true"

playback_body=$(cat <<EOF
{
  "UserId": "$CURRENT_USER_ID",
  "AutoOpenLiveStream": true,
  "MaxStreamingBitrate": 120000000,
  "DeviceProfile": $(get_device_profile_v1)
}
EOF
)

result=$(api_request "POST" "/Items/$CHANNEL_ID/PlaybackInfo" "$playback_body" "$X_EMBY_AUTH")
http_code=$(echo "$result" | cut -d'|' -f1)
response=$(echo "$result" | cut -d'|' -f2-)

if [ "$http_code" = "200" ]; then
    log_success "PlaybackInfo retrieved (HTTP $http_code)"
    save_response "playbackinfo_with_autoopen" "$response"
    
    # Extract OpenToken and check if stream was auto-opened
    open_token=$(echo "$response" | jq -r '.MediaSources[0].OpenToken // empty')
    media_source_id=$(echo "$response" | jq -r '.MediaSources[0].Id // empty')
    stream_path=$(echo "$response" | jq -r '.MediaSources[0].Path // empty')
    live_stream_id=$(echo "$response" | jq -r '.MediaSources[0].LiveStreamId // empty')
    
    if [ -n "$live_stream_id" ]; then
        log_success "AutoOpenLiveStream worked! LiveStreamId: $live_stream_id"
        if [ -n "$stream_path" ]; then
            log_success "Stream Path: $stream_path"
        fi
    elif [ -n "$open_token" ]; then
        log_info "Got OpenToken: ${open_token:0:40}..."
        log_info "Trying to open with OpenToken..."
        
        result=$(api_request "POST" "/LiveTv/LiveStreams/Open?OpenToken=$open_token" "" "$X_EMBY_AUTH")
        http_code=$(echo "$result" | cut -d'|' -f1)
        response=$(echo "$result" | cut -d'|' -f2-)
        
        if [ "$http_code" = "200" ]; then
            log_success "Opened with OpenToken (HTTP $http_code)"
            save_response "livestream_open_with_token" "$response"
            stream_path=$(echo "$response" | jq -r '.MediaSource.Path // empty')
            if [ -n "$stream_path" ]; then
                log_success "Stream Path: $stream_path"
            fi
        else
            log_error "Failed to open with OpenToken (HTTP $http_code)"
            save_response "livestream_open_token_error" "$response"
        fi
    else
        log_error "No OpenToken or LiveStreamId in response"
    fi
else
    log_error "PlaybackInfo with AutoOpenLiveStream failed (HTTP $http_code)"
fi

# Test 8: Try direct stream file endpoint (if we found a stream ID)
log_section "TEST 8: Explore LiveStreamFiles Endpoint"
log_info "This endpoint is used by Jellyfin.Xtream plugin"
log_info "Checking if we can query available streams..."

result=$(api_request "GET" "/LiveTv/LiveStreamFiles" "" "$X_EMBY_AUTH")
http_code=$(echo "$result" | cut -d'|' -f1)
response=$(echo "$result" | cut -d'|' -f2-)

if [ "$http_code" = "200" ]; then
    log_success "LiveStreamFiles endpoint accessible (HTTP $http_code)"
    save_response "livestreamfiles" "$response"
else
    log_info "LiveStreamFiles endpoint returned (HTTP $http_code)"
    save_response "livestreamfiles_response" "$response"
fi

# Test 9: Try to get channel's PlayAccess
log_section "TEST 9: GET /Items/{ChannelId}/PlaybackInfo - With StartTimeTicks"
playback_body=$(cat <<EOF
{
  "UserId": "$CURRENT_USER_ID",
  "StartTimeTicks": 0,
  "IsPlayback": true,
  "AutoOpenLiveStream": true,
  "MaxStreamingBitrate": 120000000
}
EOF
)

result=$(api_request "POST" "/Items/$CHANNEL_ID/PlaybackInfo" "$playback_body" "$X_EMBY_AUTH")
http_code=$(echo "$result" | cut -d'|' -f1)
response=$(echo "$result" | cut -d'|' -f2-)

if [ "$http_code" = "200" ]; then
    log_success "PlaybackInfo with params successful (HTTP $http_code)"
    save_response "playbackinfo_params" "$response"
else
    log_error "PlaybackInfo with params failed (HTTP $http_code)"
    save_response "playbackinfo_params_error" "$response"
fi

# Test 10: Get user's library items (to see channel structure)
log_section "TEST 10: GET /Users/{UserId}/Items - User's Library"
result=$(api_request "GET" "/Users/$CURRENT_USER_ID/Items?IncludeItemTypes=LiveTvChannel&Limit=5&Fields=Path,MediaSources,MediaStreams" "" "$X_EMBY_AUTH")
http_code=$(echo "$result" | cut -d'|' -f1)
response=$(echo "$result" | cut -d'|' -f2-)

if [ "$http_code" = "200" ]; then
    log_success "User items retrieved (HTTP $http_code)"
    save_response "user_items" "$response"
    log_info "First 5 channels structure:"
    echo "$response" | jq '.Items[] | {Name: .Name, Id: .Id, Path: .Path, HasMediaSources: (.MediaSources != null)}' | tee -a "$LOG_FILE"
else
    log_error "Failed to get user items (HTTP $http_code)"
    save_response "user_items_error" "$response"
fi

# Test 11: Record and analyze streams for each channel
if [ "$ENABLE_RECORDING" = "true" ]; then
    log_section "TEST 11: Record and Analyze Streams"
    
    # Create recordings directory
    RECORDINGS_DIR="${OUTPUT_DIR}/recordings_${TIMESTAMP}"
    mkdir -p "$RECORDINGS_DIR"
    
    for CHANNEL_ID in "${CHANNEL_ARRAY[@]}"; do
        log_info "Processing channel: $CHANNEL_ID"
        
        # Get channel name first
        result=$(api_request "GET" "/Items/$CHANNEL_ID" "" "$X_EMBY_AUTH")
        http_code=$(echo "$result" | cut -d'|' -f1)
        response=$(echo "$result" | cut -d'|' -f2-)
        
        if [ "$http_code" = "200" ]; then
            CHANNEL_NAME=$(echo "$response" | jq -r '.Name // "Unknown"' | tr ' ' '_' | tr '/' '-')
            log_info "Channel name: $CHANNEL_NAME"
        else
            CHANNEL_NAME="channel_${CHANNEL_ID:0:8}"
        fi
        
        # Open live stream with passthrough profile
        log_info "Opening live stream for $CHANNEL_NAME..."
        
        playback_body=$(cat <<EOF
{
  "UserId": "$CURRENT_USER_ID",
  "AutoOpenLiveStream": true,
  "MaxStreamingBitrate": 120000000,
  "DeviceProfile": $(get_device_profile_passthrough)
}
EOF
)
        
        result=$(api_request "POST" "/Items/$CHANNEL_ID/PlaybackInfo" "$playback_body" "$X_EMBY_AUTH")
        http_code=$(echo "$result" | cut -d'|' -f1)
        response=$(echo "$result" | cut -d'|' -f2-)
        
        if [ "$http_code" = "200" ]; then
            stream_path=$(echo "$response" | jq -r '.MediaSources[0].Path // empty')
            live_stream_id=$(echo "$response" | jq -r '.MediaSources[0].LiveStreamId // empty')
            
            if [ -n "$stream_path" ]; then
                # Replace Docker IP with actual server
                stream_url=$(echo "$stream_path" | sed "s|http://[0-9.]*:8096|${SERVER_URL}|")
                
                log_success "Got stream URL: $stream_url"
                
                # Record stream
                OUTPUT_FILE="${RECORDINGS_DIR}/${CHANNEL_NAME}_${TIMESTAMP}.ts"
                log_info "Recording $RECORD_DURATION seconds to: $OUTPUT_FILE"
                
                timeout $RECORD_DURATION curl -s "$stream_url" -o "$OUTPUT_FILE" 2>&1 | head -n 5
                
                if [ -f "$OUTPUT_FILE" ] && [ -s "$OUTPUT_FILE" ]; then
                    FILE_SIZE=$(stat -f%z "$OUTPUT_FILE" 2>/dev/null || stat -c%s "$OUTPUT_FILE" 2>/dev/null)
                    log_success "Recorded ${FILE_SIZE} bytes"
                    
                    # Run mediainfo if available
                    if command -v mediainfo &> /dev/null; then
                        log_info "Running mediainfo analysis..."
                        mediainfo "$OUTPUT_FILE" > "${RECORDINGS_DIR}/${CHANNEL_NAME}_${TIMESTAMP}.mediainfo.txt"
                        
                        # Extract key info
                        log_info "Stream details:"
                        echo "Video: $(mediainfo --Inform='Video;%Format% %Width%x%Height% %FrameRate%fps %BitRate/String%' "$OUTPUT_FILE")" | tee -a "$LOG_FILE"
                        echo "Audio: $(mediainfo --Inform='Audio;%Format% %Channel(s)%ch %SamplingRate/String% %BitRate/String%' "$OUTPUT_FILE")" | tee -a "$LOG_FILE"
                    else
                        log_info "mediainfo not installed, skipping analysis"
                        log_info "Install with: brew install mediainfo (macOS) or apt install mediainfo (Linux)"
                    fi
                else
                    log_error "Recording failed or empty file"
                fi
                
                # Close the stream
                if [ -n "$live_stream_id" ]; then
                    log_info "Closing stream..."
                    api_request "POST" "/LiveTv/LiveStreams/Close" "{\"LiveStreamId\": \"$live_stream_id\"}" "$X_EMBY_AUTH" > /dev/null
                fi
            else
                log_error "No stream path in response"
            fi
        else
            log_error "Failed to open stream (HTTP $http_code)"
        fi
        
        log ""
    done
    
    log_info "Recordings saved to: $RECORDINGS_DIR"
else
    log_info "Recording disabled (ENABLE_RECORDING=false)"
fi

# Summary
log_section "TEST SUMMARY"
log "All test results saved to: $OUTPUT_DIR"
log "Log file: $LOG_FILE"
log ""
log "Key findings:"
log "1. Check *_channel_info_*.json for MediaSources in channel data"
log "2. Check *_playbackinfo_*.json for PlaybackInfo responses"
log "3. Check *_livestream_open_*.json for successful stream opening"
log "4. Look for 'Path' fields in MediaSources - these are the stream URLs"
log ""
log_info "Analyze the JSON files to find the working API approach!"

echo ""
echo -e "${GREEN}Testing complete!${NC}"
echo "Results in: $OUTPUT_DIR"

# Commit and push results to GitHub
log_section "Pushing results to GitHub"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT"

if git rev-parse --git-dir > /dev/null 2>&1; then
    log_info "Adding test results to git..."
    git add devlog/apitest/
    
    if git diff --cached --quiet; then
        log_info "No new files to commit"
    else
        log_info "Committing test results..."
        git commit -m "Add API test results - ${TIMESTAMP}"
        
        log_info "Pushing to GitHub..."
        if git push origin main; then
            log_success "Results pushed to GitHub successfully!"
        else
            log_error "Failed to push to GitHub. You may need to push manually."
        fi
    fi
else
    log_error "Not a git repository. Results saved locally only."
fi
