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
      if (item.isMember("ChannelNumber"))
      {
        channel.number = item["ChannelNumber"].asInt();
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
  
  std::ostringstream streamUrl;
  streamUrl << m_connection->GetServerUrl() 
            << "/LiveTv/LiveStreamFiles/" << channelId 
            << "/stream.m3u8?api_key=" << m_connection->GetApiKey();
  
  kodi::addon::PVRStreamProperty prop;
  prop.SetName(PVR_STREAM_PROPERTY_STREAMURL);
  prop.SetValue(streamUrl.str());
  properties.push_back(prop);
  
  prop.SetName(PVR_STREAM_PROPERTY_ISREALTIMESTREAM);
  prop.SetValue("true");
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
