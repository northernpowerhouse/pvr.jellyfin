#pragma once

#include <string>
#include <vector>
#include <kodi/addon-instance/PVR.h>

class Connection;

struct JellyfinRecording
{
  std::string id;
  std::string title;
  std::string channelName;
  std::string plot;
  time_t startTime;
  time_t endTime;
  std::string directory;
  int playCount;
};

struct JellyfinTimer
{
  std::string id;
  std::string title;
  std::string channelId;
  time_t startTime;
  time_t endTime;
  bool isScheduled;
};

class RecordingManager
{
public:
  RecordingManager(Connection* connection, const std::string& userId);
  ~RecordingManager() = default;

  int GetRecordingCount(bool deleted) const;
  PVR_ERROR GetRecordings(bool deleted, kodi::addon::PVRRecordingsResultSet& results);
  PVR_ERROR DeleteRecording(const kodi::addon::PVRRecording& recording);
  
  int GetTimerCount() const;
  PVR_ERROR GetTimers(kodi::addon::PVRTimersResultSet& results);
  PVR_ERROR AddTimer(const kodi::addon::PVRTimer& timer);
  PVR_ERROR DeleteTimer(const kodi::addon::PVRTimer& timer);
  
  PVR_ERROR GetRecordingStreamProperties(const kodi::addon::PVRRecording& recording,
                                        std::vector<kodi::addon::PVRStreamProperty>& properties);

private:
  Connection* m_connection;
  std::string m_userId;
  std::vector<JellyfinRecording> m_recordings;
  std::vector<JellyfinTimer> m_timers;
  
  bool LoadRecordings();
  bool LoadTimers();
};
