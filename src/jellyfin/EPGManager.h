#pragma once

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <kodi/addon-instance/PVR.h>

class Connection;

struct EPGEntry
{
  std::string itemId;
  std::string channelId;
  std::string title;
  std::string plot;
  std::string episodeTitle;
  time_t startTime;
  time_t endTime;
  int parentalRating;
  int seriesNumber;
};

class EPGManager
{
public:
  EPGManager(Connection* connection, const std::string& userId);
  ~EPGManager() = default;

  PVR_ERROR GetEPGForChannel(int channelUid, time_t start, time_t end,
                            kodi::addon::PVREPGTagsResultSet& results,
                            const std::string& jellyfinChannelId);
  
  bool LoadEPGData(time_t start, time_t end);

private:
  Connection* m_connection;
  std::string m_userId;
  
  // Cache EPG data organized by channel ID
  std::map<std::string, std::vector<EPGEntry>> m_epgCache;
  time_t m_lastEPGUpdate;
};
