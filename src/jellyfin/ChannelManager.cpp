#include "ChannelManager.h"
#include "Connection.h"
#include "../utilities/Logger.h"
#include <json/json.h>
#include <sstream>
#include <functional>

ChannelManager::ChannelManager(Connection* connection, const std::string& userId)
  : m_connection(connection)
  , m_userId(userId)
{
}

bool ChannelManager::LoadChannels()
{
  Logger::Log(ADDON_LOG_INFO, "Loading channels from Jellyfin...");
  
  std::ostringstream endpoint;
  endpoint << "/LiveTv/Channels?userId=" << m_userId;
  
  Json::Value response;
  if (!m_connection->SendRequest(endpoint.str(), response))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to load channels");
    return false;
  }
  
  m_channels.clear();
  m_uidToChannelId.clear();
  
  if (response.isMember("Items") && response["Items"].isArray())
  {
    const Json::Value& items = response["Items"];
    Logger::Log(ADDON_LOG_INFO, "Processing %d channel items", items.size());
    
    for (unsigned int i = 0; i < items.size(); i++)
    {
      const Json::Value& item = items[i];
      
      // Validate required fields
      if (!item.isMember("Id") || !item.isMember("Name"))
      {
        Logger::Log(ADDON_LOG_WARNING, "Channel item %d missing required fields, skipping", i);
        continue;
      }
      
      JellyfinChannel channel;
      channel.id = item["Id"].asString();
      channel.name = item["Name"].asString();
      
      // Use ChannelNumber if available, otherwise use position
      // ChannelNumber can be a string like "1.1" or an integer
      if (item.isMember("ChannelNumber"))
      {
        if (item["ChannelNumber"].isInt())
        {
          channel.number = item["ChannelNumber"].asInt();
        }
        else if (item["ChannelNumber"].isString())
        {
          // Try to parse string as integer (e.g., "502" -> 502)
          try {
            channel.number = std::stoi(item["ChannelNumber"].asString());
          }
          catch (...) {
            // If parsing fails, use position
            channel.number = i + 1;
          }
        }
        else
        {
          channel.number = i + 1;
        }
      }
      else
      {
        channel.number = i + 1;
      }
      
      // Check channel type
      if (item.isMember("Type"))
      {
        channel.isRadio = item["Type"].asString() == "RadioChannel";
      }
      else
      {
        channel.isRadio = false;
      }
      
      if (item.isMember("ImageTags") && item["ImageTags"].isMember("Primary"))
      {
        channel.imageUrl = m_connection->GetServerUrl() + "/Items/" + channel.id + "/Images/Primary";
      }
      
      m_channels.push_back(channel);
      
      // Create UID from hash of channel ID
      std::hash<std::string> hasher;
      int uid = static_cast<int>(hasher(channel.id) & 0x7FFFFFFF);
      m_uidToChannelId[uid] = channel.id;
      
      Logger::Log(ADDON_LOG_DEBUG, "Loaded channel: %s (ID: %s, Number: %d, UID: %d)", 
                  channel.name.c_str(), channel.id.c_str(), channel.number, uid);
    }
  }
  
  Logger::Log(ADDON_LOG_INFO, "Loaded %d channels", static_cast<int>(m_channels.size()));
  
  // Load channel groups
  endpoint.str("");
  endpoint << "/LiveTv/ChannelGroups?userId=" << m_userId;
  
  if (m_connection->SendRequest(endpoint.str(), response))
  {
    m_channelGroups.clear();
    
    if (response.isMember("Items") && response["Items"].isArray())
    {
      const Json::Value& items = response["Items"];
      for (unsigned int i = 0; i < items.size(); i++)
      {
        const Json::Value& item = items[i];
        
        // Validate required fields
        if (!item.isMember("Id") || !item.isMember("Name"))
        {
          Logger::Log(ADDON_LOG_WARNING, "Channel group item %d missing required fields, skipping", i);
          continue;
        }
        
        JellyfinChannelGroup group;
        group.id = item["Id"].asString();
        group.name = item["Name"].asString();
        
        m_channelGroups.push_back(group);
      }
    }
    
    Logger::Log(ADDON_LOG_INFO, "Loaded %d channel groups", static_cast<int>(m_channelGroups.size()));
  }
  
  return true;
}

PVR_ERROR ChannelManager::GetChannels(kodi::addon::PVRChannelsResultSet& results)
{
  for (const auto& channel : m_channels)
  {
    kodi::addon::PVRChannel kodiChannel;
    
    // Find UID for this channel
    int uid = 0;
    for (const auto& pair : m_uidToChannelId)
    {
      if (pair.second == channel.id)
      {
        uid = pair.first;
        break;
      }
    }
    
    kodiChannel.SetUniqueId(uid);
    kodiChannel.SetIsRadio(channel.isRadio);
    kodiChannel.SetChannelNumber(channel.number);
    kodiChannel.SetChannelName(channel.name);
    kodiChannel.SetIconPath(channel.imageUrl);
    kodiChannel.SetIsHidden(false);
    
    results.Add(kodiChannel);
  }
  
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR ChannelManager::GetChannelGroups(kodi::addon::PVRChannelGroupsResultSet& results)
{
  for (const auto& group : m_channelGroups)
  {
    kodi::addon::PVRChannelGroup kodiGroup;
    
    std::hash<std::string> hasher;
    kodiGroup.SetGroupName(group.name);
    kodiGroup.SetIsRadio(false);
    kodiGroup.SetPosition(0);
    
    results.Add(kodiGroup);
  }
  
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR ChannelManager::GetChannelGroupMembers(const kodi::addon::PVRChannelGroup& group,
                                                  kodi::addon::PVRChannelGroupMembersResultSet& results)
{
  // Find the group
  std::string groupName = group.GetGroupName();
  JellyfinChannelGroup* jellyfinGroup = nullptr;
  
  for (auto& g : m_channelGroups)
  {
    if (g.name == groupName)
    {
      jellyfinGroup = &g;
      break;
    }
  }
  
  if (!jellyfinGroup)
    return PVR_ERROR_NO_ERROR;
  
  // If we haven't loaded the members yet, load them
  if (jellyfinGroup->channelIds.empty())
  {
    std::ostringstream endpoint;
    endpoint << "/LiveTv/Channels?userId=" << m_userId << "&groupId=" << jellyfinGroup->id;
    
    Json::Value response;
    if (m_connection->SendRequest(endpoint.str(), response))
    {
      if (response.isMember("Items") && response["Items"].isArray())
      {
        const Json::Value& items = response["Items"];
        for (unsigned int i = 0; i < items.size(); i++)
        {
          jellyfinGroup->channelIds.push_back(items[i]["Id"].asString());
        }
      }
    }
  }
  
  // Add members
  int order = 0;
  for (const auto& channelId : jellyfinGroup->channelIds)
  {
    // Find the channel UID
    for (const auto& pair : m_uidToChannelId)
    {
      if (pair.second == channelId)
      {
        kodi::addon::PVRChannelGroupMember member;
        member.SetGroupName(groupName);
        member.SetChannelUniqueId(pair.first);
        member.SetChannelNumber(++order);
        
        results.Add(member);
        break;
      }
    }
  }
  
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR ChannelManager::GetChannelStreamProperties(const kodi::addon::PVRChannel& channel,
                                                     std::vector<kodi::addon::PVRStreamProperty>& properties)
{
  std::string channelId = GetChannelIdFromUid(channel.GetUniqueId());
  if (channelId.empty())
  {
    Logger::Log(ADDON_LOG_ERROR, "Channel not found for UID: %d", channel.GetUniqueId());
    return PVR_ERROR_INVALID_PARAMETERS;
  }
  
  Logger::Log(ADDON_LOG_INFO, "Opening live stream for channel: %s", channelId.c_str());
  
  // Build DeviceProfile for live TV (matching jellyfin-kodi addon structure)
  Json::Value deviceProfile;
  deviceProfile["Name"] = "Kodi";
  deviceProfile["MaxStreamingBitrate"] = 120000000;
  deviceProfile["MaxStaticBitrate"] = 120000000;
  deviceProfile["MusicStreamingTranscodingBitrate"] = 1280000;
  deviceProfile["TimelineOffsetSeconds"] = 5;
  
  // TranscodingProfiles
  Json::Value transcodingProfiles(Json::arrayValue);
  
  // Live TV transcoding profile (HLS for live streams)
  Json::Value liveTvProfile;
  liveTvProfile["Container"] = "ts";
  liveTvProfile["Type"] = "Video";
  liveTvProfile["AudioCodec"] = "mp3,aac";
  liveTvProfile["VideoCodec"] = "h264";
  liveTvProfile["Context"] = "Streaming";
  liveTvProfile["Protocol"] = "hls";
  liveTvProfile["MaxAudioChannels"] = "2";
  liveTvProfile["MinSegments"] = "1";
  liveTvProfile["BreakOnNonKeyFrames"] = true;
  transcodingProfiles.append(liveTvProfile);
  
  // Standard video profile
  Json::Value videoProfile;
  videoProfile["Container"] = "m3u8";
  videoProfile["Type"] = "Video";
  videoProfile["AudioCodec"] = "aac,mp3,ac3,opus,flac,vorbis";
  videoProfile["VideoCodec"] = "h264,hevc,mpeg4,mpeg2video,vc1,av1";
  videoProfile["MaxAudioChannels"] = "6";
  transcodingProfiles.append(videoProfile);
  
  Json::Value audioProfile;
  audioProfile["Type"] = "Audio";
  transcodingProfiles.append(audioProfile);
  
  Json::Value photoProfile;
  photoProfile["Container"] = "jpeg";
  photoProfile["Type"] = "Photo";
  transcodingProfiles.append(photoProfile);
  
  deviceProfile["TranscodingProfiles"] = transcodingProfiles;
  
  // DirectPlayProfiles (match jellyfin-kodi addon structure)
  Json::Value directPlayProfiles(Json::arrayValue);
  Json::Value videoDirectPlay;
  videoDirectPlay["Type"] = "Video";
  videoDirectPlay["VideoCodec"] = "h264,hevc,mpeg4,mpeg2video,vc1,vp9,av1";
  directPlayProfiles.append(videoDirectPlay);
  
  Json::Value audioDirectPlay;
  audioDirectPlay["Type"] = "Audio";
  directPlayProfiles.append(audioDirectPlay);
  
  Json::Value photoDirectPlay;
  photoDirectPlay["Type"] = "Photo";
  directPlayProfiles.append(photoDirectPlay);
  
  deviceProfile["DirectPlayProfiles"] = directPlayProfiles;
  
  // Empty arrays for required fields
  deviceProfile["ResponseProfiles"] = Json::Value(Json::arrayValue);
  deviceProfile["ContainerProfiles"] = Json::Value(Json::arrayValue);
  deviceProfile["CodecProfiles"] = Json::Value(Json::arrayValue);
  deviceProfile["SubtitleProfiles"] = Json::Value(Json::arrayValue);
  
  // Build PlaybackInfo request
  Json::Value playbackInfoRequest;
  playbackInfoRequest["UserId"] = m_userId;
  playbackInfoRequest["DeviceProfile"] = deviceProfile;
  playbackInfoRequest["AutoOpenLiveStream"] = true;
  
  // Debug: Log the request JSON
  Json::StreamWriterBuilder writerBuilder;
  writerBuilder["indentation"] = "";
  std::string requestJson = Json::writeString(writerBuilder, playbackInfoRequest);
  Logger::Log(ADDON_LOG_DEBUG, "PlaybackInfo request JSON length: %zu bytes", requestJson.length());
  
  // POST /Items/{id}/PlaybackInfo
  std::string playbackInfoUrl = "/Items/" + channelId + "/PlaybackInfo";
  Json::Value playbackInfo;
  
  if (!m_connection->SendPostRequest(playbackInfoUrl, playbackInfoRequest, playbackInfo))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to get PlaybackInfo for channel: %s", channelId.c_str());
    Logger::Log(ADDON_LOG_ERROR, "Request was: %s", requestJson.c_str());
    return PVR_ERROR_SERVER_ERROR;
  }
  
  // Extract MediaSources[0] with LiveStreamId and MediaSourceId
  if (!playbackInfo.isMember("MediaSources") || !playbackInfo["MediaSources"].isArray() || playbackInfo["MediaSources"].size() == 0)
  {
    Logger::Log(ADDON_LOG_ERROR, "No MediaSources in PlaybackInfo response");
    return PVR_ERROR_SERVER_ERROR;
  }
  
  Json::Value mediaSource = playbackInfo["MediaSources"][0];
  std::string liveStreamId = mediaSource.get("LiveStreamId", "").asString();
  std::string mediaSourceId = mediaSource.get("Id", channelId).asString();
  std::string streamPath = mediaSource.get("Path", "").asString();
  
  if (liveStreamId.empty())
  {
    Logger::Log(ADDON_LOG_ERROR, "No LiveStreamId in MediaSources response");
    return PVR_ERROR_SERVER_ERROR;
  }
  
  Logger::Log(ADDON_LOG_INFO, "Got LiveStreamId: %s, MediaSourceId: %s", liveStreamId.c_str(), mediaSourceId.c_str());
  
  // Build stream URL - prefer Path from MediaSources if provided
  std::string streamUrl;
  if (!streamPath.empty())
  {
    // Server provided a direct path (e.g., http://172.23.0.2:8096/LiveTv/LiveStreamFiles/{guid}/stream.ts)
    Logger::Log(ADDON_LOG_INFO, "Using server-provided stream path: %s", streamPath.c_str());
    
    // Replace the server URL in the path with our configured server URL
    // (server might return internal Docker IP like 172.23.0.2)
    std::string serverUrl = m_connection->GetServerUrl();
    
    // Find the path part after the server URL
    size_t pathStart = streamPath.find("/LiveTv");
    if (pathStart == std::string::npos)
    {
      pathStart = streamPath.find("/Videos");
    }
    
    if (pathStart != std::string::npos)
    {
      // Extract just the path and append to our server URL
      std::string pathOnly = streamPath.substr(pathStart);
      streamUrl = serverUrl + pathOnly;
      
      // Add API key if not already in the path
      if (streamUrl.find("api_key=") == std::string::npos)
      {
        streamUrl += (streamUrl.find('?') != std::string::npos ? "&" : "?");
        streamUrl += "api_key=" + m_connection->GetApiKey();
      }
      
      Logger::Log(ADDON_LOG_INFO, "Adjusted stream URL: %s", streamUrl.c_str());
    }
    else
    {
      // Path doesn't match expected format, use as-is
      streamUrl = streamPath;
    }
  }
  else
  {
    // Fallback: Build URL using live.m3u8 with parameters
    std::ostringstream urlBuilder;
    urlBuilder << m_connection->GetServerUrl() 
              << "/videos/" << channelId 
              << "/live.m3u8?LiveStreamId=" << liveStreamId
              << "&MediaSourceId=" << mediaSourceId
              << "&api_key=" << m_connection->GetApiKey();
    streamUrl = urlBuilder.str();
    Logger::Log(ADDON_LOG_INFO, "Built stream URL: %s", streamUrl.c_str());
  }
  
  kodi::addon::PVRStreamProperty prop;
  prop.SetName(PVR_STREAM_PROPERTY_STREAMURL);
  prop.SetValue(streamUrl);
  properties.push_back(prop);
  
  prop.SetName(PVR_STREAM_PROPERTY_ISREALTIMESTREAM);
  prop.SetValue("true");
  properties.push_back(prop);
  
  prop.SetName(PVR_STREAM_PROPERTY_MIMETYPE);
  prop.SetValue("application/x-mpegURL");
  properties.push_back(prop);
  
  return PVR_ERROR_NO_ERROR;
}

std::string ChannelManager::GetChannelIdFromUid(int uid) const
{
  auto it = m_uidToChannelId.find(uid);
  if (it != m_uidToChannelId.end())
    return it->second;
  return "";
}
