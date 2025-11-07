#include "EPGManager.h"
#include "Connection.h"
#include "../utilities/Logger.h"
#include "../utilities/Utilities.h"
#include <json/json.h>
#include <sstream>
#include <chrono>

EPGManager::EPGManager(Connection* connection, const std::string& userId)
  : m_connection(connection)
  , m_userId(userId)
  , m_lastEPGUpdate(0)
{
}

bool EPGManager::LoadEPGData(time_t start, time_t end)
{
  Logger::Log(ADDON_LOG_INFO, "Loading EPG data from %s to %s", 
              Utilities::FormatDateTime(start).c_str(),
              Utilities::FormatDateTime(end).c_str());
  
  // Make ONE bulk API call for all channels
  std::ostringstream endpoint;
  endpoint << "/LiveTv/Programs?userId=" << m_userId
           << "&minStartDate=" << Utilities::FormatDateTime(start)
           << "&maxStartDate=" << Utilities::FormatDateTime(end);
  
  Json::Value response;
  if (!m_connection->SendRequest(endpoint.str(), response))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to load EPG data");
    return false;
  }
  
  // Clear old cache
  m_epgCache.clear();
  
  if (response.isMember("Items") && response["Items"].isArray())
  {
    const Json::Value& items = response["Items"];
    Logger::Log(ADDON_LOG_INFO, "Processing %d EPG items", items.size());
    
    for (unsigned int i = 0; i < items.size(); i++)
    {
      const Json::Value& item = items[i];
      
      // Validate required fields
      if (!item.isMember("Id") || !item.isMember("ChannelId"))
      {
        continue;
      }
      
      std::string channelId = item["ChannelId"].asString();
      
      EPGEntry entry;
      entry.itemId = item["Id"].asString();
      entry.channelId = channelId;
      entry.title = item.get("Name", "").asString();
      entry.plot = item.get("Overview", "").asString();
      entry.episodeTitle = item.get("EpisodeTitle", "").asString();
      
      if (item.isMember("StartDate"))
      {
        entry.startTime = Utilities::ParseDateTime(item["StartDate"].asString());
      }
      
      if (item.isMember("EndDate"))
      {
        entry.endTime = Utilities::ParseDateTime(item["EndDate"].asString());
      }
      
      if (item.isMember("ParentalRating"))
      {
        entry.parentalRating = item["ParentalRating"].asInt();
      }
      else
      {
        entry.parentalRating = 0;
      }
      
      if (item.isMember("SeriesId") && item.isMember("IndexNumber"))
      {
        entry.seriesNumber = item["IndexNumber"].asInt();
      }
      else
      {
        entry.seriesNumber = 0;
      }
      
      // Store in cache organized by channel ID
      m_epgCache[channelId].push_back(entry);
    }
  }
  
  m_lastEPGUpdate = std::time(nullptr);
  Logger::Log(ADDON_LOG_INFO, "Loaded EPG data for %d channels", static_cast<int>(m_epgCache.size()));
  
  return true;
}

PVR_ERROR EPGManager::GetEPGForChannel(int channelUid, time_t start, time_t end,
                                       kodi::addon::PVREPGTagsResultSet& results,
                                       const std::string& jellyfinChannelId)
{
  // Check if we need to refresh the cache
  time_t now = std::time(nullptr);
  if (m_epgCache.empty() || (now - m_lastEPGUpdate) > 3600) // Refresh every hour
  {
    if (!LoadEPGData(start, end))
    {
      return PVR_ERROR_SERVER_ERROR;
    }
  }
  
  // Find entries for this specific channel from cache
  auto it = m_epgCache.find(jellyfinChannelId);
  if (it == m_epgCache.end())
  {
    // No EPG data for this channel
    return PVR_ERROR_NO_ERROR;
  }
  
  int addedCount = 0;
  
  for (const auto& entry : it->second)
  {
    kodi::addon::PVREPGTag tag;
    
    // Generate unique broadcast ID from hash
    std::hash<std::string> hasher;
    unsigned int broadcastId = static_cast<unsigned int>(hasher(entry.itemId));
    
    tag.SetUniqueBroadcastId(broadcastId);
    tag.SetUniqueChannelId(channelUid);
    tag.SetTitle(entry.title);
    tag.SetPlot(entry.plot);
    tag.SetStartTime(entry.startTime);
    tag.SetEndTime(entry.endTime);
    
    if (!entry.episodeTitle.empty())
    {
      tag.SetEpisodeName(entry.episodeTitle);
    }
    
    if (entry.parentalRating > 0)
    {
      tag.SetParentalRating(entry.parentalRating);
    }
    
    if (entry.seriesNumber > 0)
    {
      tag.SetSeriesNumber(entry.seriesNumber);
    }
    
    results.Add(tag);
    addedCount++;
  }
  
  Logger::Log(ADDON_LOG_DEBUG, "Added %d EPG entries for channel UID %d (%s)", 
              addedCount, channelUid, jellyfinChannelId.c_str());
  
  return PVR_ERROR_NO_ERROR;
}
