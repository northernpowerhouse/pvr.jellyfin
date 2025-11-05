#include "RecordingManager.h"
#include "Connection.h"
#include "../utilities/Logger.h"
#include "../utilities/Utilities.h"
#include <json/json.h>
#include <sstream>

RecordingManager::RecordingManager(Connection* connection, const std::string& userId)
  : m_connection(connection)
  , m_userId(userId)
{
}

bool RecordingManager::LoadRecordings()
{
  Logger::Log(ADDON_LOG_INFO, "Loading recordings from Jellyfin...");
  
  std::ostringstream endpoint;
  endpoint << "/LiveTv/Recordings?userId=" << m_userId;
  
  Json::Value response;
  if (!m_connection->SendRequest(endpoint.str(), response))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to load recordings");
    return false;
  }
  
  m_recordings.clear();
  
  if (response.isMember("Items") && response["Items"].isArray())
  {
    const Json::Value& items = response["Items"];
    for (unsigned int i = 0; i < items.size(); i++)
    {
      const Json::Value& item = items[i];
      
      JellyfinRecording recording;
      recording.id = item["Id"].asString();
      recording.title = item.get("Name", "").asString();
      recording.channelName = item.get("ChannelName", "").asString();
      recording.plot = item.get("Overview", "").asString();
      recording.playCount = item.get("UserData", Json::Value::null).get("PlayCount", 0).asInt();
      
      if (item.isMember("StartDate"))
      {
        recording.startTime = Utilities::ParseDateTime(item["StartDate"].asString());
      }
      
      if (item.isMember("EndDate"))
      {
        recording.endTime = Utilities::ParseDateTime(item["EndDate"].asString());
      }
      
      recording.directory = item.get("SeriesName", "").asString();
      
      m_recordings.push_back(recording);
    }
  }
  
  Logger::Log(ADDON_LOG_INFO, "Loaded %d recordings", static_cast<int>(m_recordings.size()));
  return true;
}

bool RecordingManager::LoadTimers()
{
  Logger::Log(ADDON_LOG_INFO, "Loading timers from Jellyfin...");
  
  std::ostringstream endpoint;
  endpoint << "/LiveTv/Timers?userId=" << m_userId;
  
  Json::Value response;
  if (!m_connection->SendRequest(endpoint.str(), response))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to load timers");
    return false;
  }
  
  m_timers.clear();
  
  if (response.isMember("Items") && response["Items"].isArray())
  {
    const Json::Value& items = response["Items"];
    for (unsigned int i = 0; i < items.size(); i++)
    {
      const Json::Value& item = items[i];
      
      JellyfinTimer timer;
      timer.id = item["Id"].asString();
      timer.title = item.get("Name", "").asString();
      timer.channelId = item.get("ChannelId", "").asString();
      timer.isScheduled = item.get("Status", "").asString() == "New";
      
      if (item.isMember("StartDate"))
      {
        timer.startTime = Utilities::ParseDateTime(item["StartDate"].asString());
      }
      
      if (item.isMember("EndDate"))
      {
        timer.endTime = Utilities::ParseDateTime(item["EndDate"].asString());
      }
      
      m_timers.push_back(timer);
    }
  }
  
  Logger::Log(ADDON_LOG_INFO, "Loaded %d timers", static_cast<int>(m_timers.size()));
  return true;
}

int RecordingManager::GetRecordingCount(bool deleted) const
{
  // Jellyfin doesn't have a "deleted" state for recordings
  if (deleted)
    return 0;
  
  return m_recordings.size();
}

PVR_ERROR RecordingManager::GetRecordings(bool deleted, kodi::addon::PVRRecordingsResultSet& results)
{
  if (deleted)
    return PVR_ERROR_NO_ERROR;
  
  // Refresh recordings
  if (!LoadRecordings())
    return PVR_ERROR_SERVER_ERROR;
  
  for (const auto& recording : m_recordings)
  {
    kodi::addon::PVRRecording kodiRecording;
    
    kodiRecording.SetRecordingId(recording.id);
    kodiRecording.SetTitle(recording.title);
    kodiRecording.SetPlot(recording.plot);
    kodiRecording.SetChannelName(recording.channelName);
    kodiRecording.SetRecordingTime(recording.startTime);
    kodiRecording.SetDuration(static_cast<int>(recording.endTime - recording.startTime));
    kodiRecording.SetPlayCount(recording.playCount);
    kodiRecording.SetDirectory(recording.directory);
    
    results.Add(kodiRecording);
  }
  
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR RecordingManager::DeleteRecording(const kodi::addon::PVRRecording& recording)
{
  std::string recordingId = recording.GetRecordingId();
  
  std::ostringstream endpoint;
  endpoint << "/LiveTv/Recordings/" << recordingId;
  
  if (!m_connection->SendDeleteRequest(endpoint.str()))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to delete recording: %s", recordingId.c_str());
    return PVR_ERROR_SERVER_ERROR;
  }
  
  Logger::Log(ADDON_LOG_INFO, "Deleted recording: %s", recordingId.c_str());
  return PVR_ERROR_NO_ERROR;
}

int RecordingManager::GetTimerCount() const
{
  return m_timers.size();
}

PVR_ERROR RecordingManager::GetTimers(kodi::addon::PVRTimersResultSet& results)
{
  // Refresh timers
  if (!LoadTimers())
    return PVR_ERROR_SERVER_ERROR;
  
  for (const auto& timer : m_timers)
  {
    kodi::addon::PVRTimer kodiTimer;
    
    std::hash<std::string> hasher;
    unsigned int timerId = static_cast<unsigned int>(hasher(timer.id));
    
    kodiTimer.SetClientIndex(timerId);
    kodiTimer.SetTitle(timer.title);
    kodiTimer.SetStartTime(timer.startTime);
    kodiTimer.SetEndTime(timer.endTime);
    kodiTimer.SetState(timer.isScheduled ? PVR_TIMER_STATE_SCHEDULED : PVR_TIMER_STATE_RECORDING);
    
    // Map channel ID to channel UID (simplified - would need proper mapping)
    kodiTimer.SetClientChannelUid(0);
    
    results.Add(kodiTimer);
  }
  
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR RecordingManager::AddTimer(const kodi::addon::PVRTimer& timer)
{
  Json::Value timerData;
  timerData["Name"] = timer.GetTitle();
  timerData["StartDate"] = Utilities::FormatDateTime(timer.GetStartTime());
  timerData["EndDate"] = Utilities::FormatDateTime(timer.GetEndTime());
  // Would need to map channel UID to channel ID
  // timerData["ChannelId"] = ...;
  
  Json::Value response;
  if (!m_connection->SendPostRequest("/LiveTv/Timers", timerData, response))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to add timer");
    return PVR_ERROR_SERVER_ERROR;
  }
  
  Logger::Log(ADDON_LOG_INFO, "Added timer: %s", timer.GetTitle().c_str());
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR RecordingManager::DeleteTimer(const kodi::addon::PVRTimer& timer)
{
  // Find timer by client index
  std::string timerId;
  std::hash<std::string> hasher;
  
  for (const auto& t : m_timers)
  {
    unsigned int id = static_cast<unsigned int>(hasher(t.id));
    if (id == timer.GetClientIndex())
    {
      timerId = t.id;
      break;
    }
  }
  
  if (timerId.empty())
  {
    Logger::Log(ADDON_LOG_ERROR, "Timer not found");
    return PVR_ERROR_INVALID_PARAMETERS;
  }
  
  std::ostringstream endpoint;
  endpoint << "/LiveTv/Timers/" << timerId;
  
  if (!m_connection->SendDeleteRequest(endpoint.str()))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to delete timer: %s", timerId.c_str());
    return PVR_ERROR_SERVER_ERROR;
  }
  
  Logger::Log(ADDON_LOG_INFO, "Deleted timer: %s", timerId.c_str());
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR RecordingManager::GetRecordingStreamProperties(const kodi::addon::PVRRecording& recording,
                                                         std::vector<kodi::addon::PVRStreamProperty>& properties)
{
  std::string recordingId = recording.GetRecordingId();
  
  std::ostringstream streamUrl;
  streamUrl << m_connection->GetServerUrl() 
            << "/Videos/" << recordingId 
            << "/stream?static=true&api_key=" << m_connection->GetApiKey();
  
  kodi::addon::PVRStreamProperty prop;
  prop.SetName(PVR_STREAM_PROPERTY_STREAMURL);
  prop.SetValue(streamUrl.str());
  properties.push_back(prop);
  
  return PVR_ERROR_NO_ERROR;
}
