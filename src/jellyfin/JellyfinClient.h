#pragma once

#include <string>
#include <memory>
#include <kodi/addon-instance/PVR.h>

class Connection;
class ChannelManager;
class EPGManager;
class RecordingManager;

class JellyfinClient
{
public:
  JellyfinClient(const std::string& serverUrl, const std::string& userId, const std::string& apiKey);
  ~JellyfinClient();

  bool Connect();
  std::string GetServerVersion() const { return m_serverVersion; }
  
  // Channel operations
  int GetChannelCount() const;
  PVR_ERROR GetChannels(kodi::addon::PVRChannelsResultSet& results);
  int GetChannelGroupCount() const;
  PVR_ERROR GetChannelGroups(kodi::addon::PVRChannelGroupsResultSet& results);
  PVR_ERROR GetChannelGroupMembers(const kodi::addon::PVRChannelGroup& group,
                                   kodi::addon::PVRChannelGroupMembersResultSet& results);
  
  // EPG operations
  PVR_ERROR GetEPGForChannel(int channelUid, time_t start, time_t end,
                            kodi::addon::PVREPGTagsResultSet& results);
  
  // Recording operations
  int GetRecordingCount(bool deleted) const;
  PVR_ERROR GetRecordings(bool deleted, kodi::addon::PVRRecordingsResultSet& results);
  PVR_ERROR DeleteRecording(const kodi::addon::PVRRecording& recording);
  
  // Timer operations
  int GetTimerCount() const;
  PVR_ERROR GetTimers(kodi::addon::PVRTimersResultSet& results);
  PVR_ERROR AddTimer(const kodi::addon::PVRTimer& timer);
  PVR_ERROR DeleteTimer(const kodi::addon::PVRTimer& timer);
  
  // Stream operations
  PVR_ERROR GetChannelStreamProperties(const kodi::addon::PVRChannel& channel,
                                      std::vector<kodi::addon::PVRStreamProperty>& properties);
  PVR_ERROR GetRecordingStreamProperties(const kodi::addon::PVRRecording& recording,
                                        std::vector<kodi::addon::PVRStreamProperty>& properties);

private:
  std::string m_serverUrl;
  std::string m_userId;
  std::string m_apiKey;
  std::string m_serverVersion;
  
  std::unique_ptr<Connection> m_connection;
  std::unique_ptr<ChannelManager> m_channelManager;
  std::unique_ptr<EPGManager> m_epgManager;
  std::unique_ptr<RecordingManager> m_recordingManager;
};
