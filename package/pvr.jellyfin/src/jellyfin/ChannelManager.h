#pragma once

#include <string>
#include <vector>
#include <map>
#include <kodi/addon-instance/PVR.h>

class Connection;

struct JellyfinChannel
{
  std::string id;
  std::string name;
  int number;
  std::string imageUrl;
  bool isRadio;
};

struct JellyfinChannelGroup
{
  std::string id;
  std::string name;
  std::vector<std::string> channelIds;
};

class ChannelManager
{
public:
  ChannelManager(Connection* connection, const std::string& userId);
  ~ChannelManager() = default;

  bool LoadChannels();
  int GetChannelCount() const { return m_channels.size(); }
  PVR_ERROR GetChannels(kodi::addon::PVRChannelsResultSet& results);
  
  int GetChannelGroupCount() const { return m_channelGroups.size(); }
  PVR_ERROR GetChannelGroups(kodi::addon::PVRChannelGroupsResultSet& results);
  PVR_ERROR GetChannelGroupMembers(const kodi::addon::PVRChannelGroup& group,
                                   kodi::addon::PVRChannelGroupMembersResultSet& results);
  
  PVR_ERROR GetChannelStreamProperties(const kodi::addon::PVRChannel& channel,
                                      std::vector<kodi::addon::PVRStreamProperty>& properties);
  
  std::string GetChannelIdFromUid(int uid) const;

private:
  Connection* m_connection;
  std::string m_userId;
  std::vector<JellyfinChannel> m_channels;
  std::vector<JellyfinChannelGroup> m_channelGroups;
  std::map<int, std::string> m_uidToChannelId;
};
