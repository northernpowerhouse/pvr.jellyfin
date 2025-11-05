#include "EPGManager.h"
#include "Connection.h"
#include "../utilities/Logger.h"
#include "../utilities/Utilities.h"
#include <json/json.h>
#include <sstream>

EPGManager::EPGManager(Connection* connection, const std::string& userId)
  : m_connection(connection)
  , m_userId(userId)
{
}

PVR_ERROR EPGManager::GetEPGForChannel(int channelUid, time_t start, time_t end,
                                       kodi::addon::PVREPGTagsResultSet& results)
{
  // Note: In a real implementation, we'd need to map channelUid back to Jellyfin channel ID
  // For now, we'll construct a simple request
  
  std::ostringstream endpoint;
  endpoint << "/LiveTv/Programs?userId=" << m_userId
           << "&minStartDate=" << Utilities::FormatDateTime(start)
           << "&maxStartDate=" << Utilities::FormatDateTime(end);
  
  Json::Value response;
  if (!m_connection->SendRequest(endpoint.str(), response))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to load EPG data");
    return PVR_ERROR_SERVER_ERROR;
  }
  
  if (response.isMember("Items") && response["Items"].isArray())
  {
    const Json::Value& items = response["Items"];
    
    for (unsigned int i = 0; i < items.size(); i++)
    {
      const Json::Value& item = items[i];
      
      // Extract channel info - we need to match against our channelUid
      if (!item.isMember("ChannelId"))
        continue;
      
      std::string channelId = item["ChannelId"].asString();
      // TODO: Map channelId to channelUid and filter
      
      kodi::addon::PVREPGTag tag;
      
      // Generate unique broadcast ID from hash
      std::string itemId = item["Id"].asString();
      std::hash<std::string> hasher;
      unsigned int broadcastId = static_cast<unsigned int>(hasher(itemId));
      
      tag.SetUniqueBroadcastId(broadcastId);
      tag.SetUniqueChannelId(channelUid);
      tag.SetTitle(item.get("Name", "").asString());
      tag.SetPlot(item.get("Overview", "").asString());
      
      if (item.isMember("StartDate"))
      {
        time_t startTime = Utilities::ParseDateTime(item["StartDate"].asString());
        tag.SetStartTime(startTime);
      }
      
      if (item.isMember("EndDate"))
      {
        time_t endTime = Utilities::ParseDateTime(item["EndDate"].asString());
        tag.SetEndTime(endTime);
      }
      
      if (item.isMember("EpisodeTitle"))
      {
        tag.SetEpisodeName(item["EpisodeTitle"].asString());
      }
      
      if (item.isMember("ParentalRating"))
      {
        tag.SetParentalRating(item["ParentalRating"].asInt());
      }
      
      if (item.isMember("SeriesId"))
      {
        tag.SetSeriesNumber(item.get("IndexNumber", 0).asInt());
      }
      
      results.Add(tag);
    }
  }
  
  return PVR_ERROR_NO_ERROR;
}
