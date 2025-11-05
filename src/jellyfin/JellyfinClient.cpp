#include "JellyfinClient.h"
#include "Connection.h"
#include "ChannelManager.h"
#include "EPGManager.h"
#include "RecordingManager.h"
#include "../utilities/Logger.h"
#include <json/json.h>

JellyfinClient::JellyfinClient(const std::string& serverUrl, const std::string& userId, const std::string& apiKey)
  : m_serverUrl(serverUrl)
  , m_userId(userId)
  , m_apiKey(apiKey)
  , m_serverVersion("Unknown")
{
  m_connection = std::make_unique<Connection>(serverUrl, apiKey);
}

JellyfinClient::~JellyfinClient() = default;

bool JellyfinClient::Connect()
{
  Logger::Log(ADDON_LOG_INFO, "Connecting to Jellyfin server at %s", m_serverUrl.c_str());
  
  // Get server info to verify connection
  Json::Value response;
  if (!m_connection->SendRequest("/System/Info", response))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to get server info");
    return false;
  }
  
  if (response.isMember("Version"))
  {
    m_serverVersion = response["Version"].asString();
    Logger::Log(ADDON_LOG_INFO, "Connected to Jellyfin server version %s", m_serverVersion.c_str());
  }
  
  // Initialize managers
  m_channelManager = std::make_unique<ChannelManager>(m_connection.get(), m_userId);
  m_epgManager = std::make_unique<EPGManager>(m_connection.get(), m_userId);
  m_recordingManager = std::make_unique<RecordingManager>(m_connection.get(), m_userId);
  
  // Load initial data
  m_channelManager->LoadChannels();
  
  return true;
}

int JellyfinClient::GetChannelCount() const
{
  if (m_channelManager)
    return m_channelManager->GetChannelCount();
  return 0;
}

PVR_ERROR JellyfinClient::GetChannels(kodi::addon::PVRChannelsResultSet& results)
{
  if (m_channelManager)
    return m_channelManager->GetChannels(results);
  return PVR_ERROR_SERVER_ERROR;
}

int JellyfinClient::GetChannelGroupCount() const
{
  if (m_channelManager)
    return m_channelManager->GetChannelGroupCount();
  return 0;
}

PVR_ERROR JellyfinClient::GetChannelGroups(kodi::addon::PVRChannelGroupsResultSet& results)
{
  if (m_channelManager)
    return m_channelManager->GetChannelGroups(results);
  return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR JellyfinClient::GetChannelGroupMembers(const kodi::addon::PVRChannelGroup& group,
                                                 kodi::addon::PVRChannelGroupMembersResultSet& results)
{
  if (m_channelManager)
    return m_channelManager->GetChannelGroupMembers(group, results);
  return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR JellyfinClient::GetEPGForChannel(int channelUid, time_t start, time_t end,
                                           kodi::addon::PVREPGTagsResultSet& results)
{
  if (m_epgManager)
    return m_epgManager->GetEPGForChannel(channelUid, start, end, results);
  return PVR_ERROR_SERVER_ERROR;
}

int JellyfinClient::GetRecordingCount(bool deleted) const
{
  if (m_recordingManager)
    return m_recordingManager->GetRecordingCount(deleted);
  return 0;
}

PVR_ERROR JellyfinClient::GetRecordings(bool deleted, kodi::addon::PVRRecordingsResultSet& results)
{
  if (m_recordingManager)
    return m_recordingManager->GetRecordings(deleted, results);
  return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR JellyfinClient::DeleteRecording(const kodi::addon::PVRRecording& recording)
{
  if (m_recordingManager)
    return m_recordingManager->DeleteRecording(recording);
  return PVR_ERROR_SERVER_ERROR;
}

int JellyfinClient::GetTimerCount() const
{
  if (m_recordingManager)
    return m_recordingManager->GetTimerCount();
  return 0;
}

PVR_ERROR JellyfinClient::GetTimers(kodi::addon::PVRTimersResultSet& results)
{
  if (m_recordingManager)
    return m_recordingManager->GetTimers(results);
  return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR JellyfinClient::AddTimer(const kodi::addon::PVRTimer& timer)
{
  if (m_recordingManager)
    return m_recordingManager->AddTimer(timer);
  return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR JellyfinClient::DeleteTimer(const kodi::addon::PVRTimer& timer)
{
  if (m_recordingManager)
    return m_recordingManager->DeleteTimer(timer);
  return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR JellyfinClient::GetChannelStreamProperties(const kodi::addon::PVRChannel& channel,
                                                     std::vector<kodi::addon::PVRStreamProperty>& properties)
{
  if (m_channelManager)
    return m_channelManager->GetChannelStreamProperties(channel, properties);
  return PVR_ERROR_SERVER_ERROR;
}

PVR_ERROR JellyfinClient::GetRecordingStreamProperties(const kodi::addon::PVRRecording& recording,
                                                       std::vector<kodi::addon::PVRStreamProperty>& properties)
{
  if (m_recordingManager)
    return m_recordingManager->GetRecordingStreamProperties(recording, properties);
  return PVR_ERROR_SERVER_ERROR;
}
