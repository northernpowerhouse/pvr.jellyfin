#pragma once

#include <string>
#include <kodi/addon-instance/PVR.h>

class Connection;

class EPGManager
{
public:
  EPGManager(Connection* connection, const std::string& userId);
  ~EPGManager() = default;

  PVR_ERROR GetEPGForChannel(int channelUid, time_t start, time_t end,
                            kodi::addon::PVREPGTagsResultSet& results);

private:
  Connection* m_connection;
  std::string m_userId;
};
