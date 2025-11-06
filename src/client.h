#pragma once

#include <kodi/addon-instance/PVR.h>
#include <memory>

class CJellyfinPVRClient;

class ATTR_DLL_LOCAL CJellyfinAddon : public kodi::addon::CAddonBase
{
public:
  CJellyfinAddon() = default;
  ADDON_STATUS CreateInstance(const kodi::addon::IInstanceInfo& instance,
                              KODI_ADDON_INSTANCE_HDL& hdl) override;
};

class ATTR_DLL_LOCAL CJellyfinPVRClient : public kodi::addon::CInstancePVRClient
{
public:
  CJellyfinPVRClient(const kodi::addon::IInstanceInfo& instance);
  ~CJellyfinPVRClient() override;

  // Connection
  PVR_ERROR GetCapabilities(kodi::addon::PVRCapabilities& capabilities) override;
  PVR_ERROR GetBackendName(std::string& name) override;
  PVR_ERROR GetBackendVersion(std::string& version) override;
  PVR_ERROR GetConnectionString(std::string& connection) override;
  PVR_ERROR GetBackendHostname(std::string& hostname) override;

  // Channels
  PVR_ERROR GetChannelsAmount(int& amount) override;
  PVR_ERROR GetChannels(bool radio, kodi::addon::PVRChannelsResultSet& results) override;
  PVR_ERROR GetChannelGroupsAmount(int& amount) override;
  PVR_ERROR GetChannelGroups(bool radio, kodi::addon::PVRChannelGroupsResultSet& results) override;
  PVR_ERROR GetChannelGroupMembers(const kodi::addon::PVRChannelGroup& group,
                                   kodi::addon::PVRChannelGroupMembersResultSet& results) override;

  // EPG
  PVR_ERROR GetEPGForChannel(int channelUid,
                            time_t start,
                            time_t end,
                            kodi::addon::PVREPGTagsResultSet& results) override;

  // Recordings
  PVR_ERROR GetRecordingsAmount(bool deleted, int& amount) override;
  PVR_ERROR GetRecordings(bool deleted, kodi::addon::PVRRecordingsResultSet& results) override;
  PVR_ERROR DeleteRecording(const kodi::addon::PVRRecording& recording) override;

  // Timers (scheduled recordings)
  PVR_ERROR GetTimersAmount(int& amount) override;
  PVR_ERROR GetTimers(kodi::addon::PVRTimersResultSet& results) override;
  PVR_ERROR AddTimer(const kodi::addon::PVRTimer& timer) override;
  PVR_ERROR DeleteTimer(const kodi::addon::PVRTimer& timer, bool forceDelete) override;

  // Stream URLs
  PVR_ERROR GetChannelStreamProperties(const kodi::addon::PVRChannel& channel,
                                       std::vector<kodi::addon::PVRStreamProperty>& properties) override;
  PVR_ERROR GetRecordingStreamProperties(const kodi::addon::PVRRecording& recording,
                                         std::vector<kodi::addon::PVRStreamProperty>& properties) override;

private:
  std::unique_ptr<class JellyfinClient> m_jellyfinClient;
  bool LoadSettings();
  
  std::string m_serverUrl;
  std::string m_userId;
  std::string m_apiKey;
};
