#include "client.h"
#include "jellyfin/JellyfinClient.h"
#include "utilities/Logger.h"
#include <kodi/General.h>
#include <sstream>

ADDON_STATUS CJellyfinAddon::CreateInstance(const kodi::addon::IInstanceInfo& instance,
                                            KODI_ADDON_INSTANCE_HDL& hdl)
{
  if (instance.IsType(ADDON_INSTANCE_PVR))
  {
    hdl = new CJellyfinPVRClient(instance);
    return ADDON_STATUS_OK;
  }
  return ADDON_STATUS_UNKNOWN;
}

CJellyfinPVRClient::CJellyfinPVRClient(const kodi::addon::IInstanceInfo& instance)
  : CInstancePVRClient(instance)
{
  Logger::Log(ADDON_LOG_INFO, "Jellyfin PVR Client starting...");
  
  if (LoadSettings())
  {
    m_jellyfinClient = std::make_unique<JellyfinClient>(m_serverUrl, m_userId, m_apiKey);
    
    // Try to initialize with existing credentials first
    if (m_jellyfinClient->Initialize())
    {
      Logger::Log(ADDON_LOG_INFO, "Successfully connected to Jellyfin server");
    }
    else
    {
      // Need to authenticate
      Logger::Log(ADDON_LOG_INFO, "Authentication required");
      
      int authMethod = kodi::addon::GetSettingInt("auth_method", 0);
      
      if (authMethod == 0) // Username & Password
      {
        std::string username = kodi::addon::GetSettingString("username", "");
        std::string password = kodi::addon::GetSettingString("password", "");
        
        if (!username.empty() && !password.empty())
        {
          if (m_jellyfinClient->AuthenticateWithPassword(username, password))
          {
            Logger::Log(ADDON_LOG_INFO, "Successfully authenticated with password");
          }
          else
          {
            Logger::Log(ADDON_LOG_ERROR, "Failed to authenticate with password");
          }
        }
        else
        {
          Logger::Log(ADDON_LOG_WARNING, "Username or password not configured");
        }
      }
      else if (authMethod == 1) // Quick Connect
      {
        if (m_jellyfinClient->AuthenticateWithQuickConnect())
        {
          Logger::Log(ADDON_LOG_INFO, "Successfully authenticated with Quick Connect");
        }
        else
        {
          Logger::Log(ADDON_LOG_ERROR, "Quick Connect authentication failed");
        }
      }
      else // Manual API Key (auth_method == 2)
      {
        if (!m_apiKey.empty())
        {
          if (m_jellyfinClient->Connect())
          {
            Logger::Log(ADDON_LOG_INFO, "Successfully connected with API key");
          }
          else
          {
            Logger::Log(ADDON_LOG_ERROR, "Failed to connect with API key");
          }
        }
        else
        {
          Logger::Log(ADDON_LOG_WARNING, "API key not configured");
        }
      }
    }
  }
}

CJellyfinPVRClient::~CJellyfinPVRClient()
{
  Logger::Log(ADDON_LOG_INFO, "Jellyfin PVR Client shutting down...");
}

bool CJellyfinPVRClient::LoadSettings()
{
  // Build server URL from components
  bool useHttps = kodi::addon::GetSettingBoolean("use_https", false);
  std::string serverAddress = kodi::addon::GetSettingString("server_address", "");
  int serverPort = kodi::addon::GetSettingInt("server_port", 8096);
  
  if (!serverAddress.empty())
  {
    std::ostringstream urlBuilder;
    urlBuilder << (useHttps ? "https://" : "http://") << serverAddress;
    
    // Only add port if it's not the default for the protocol
    if ((useHttps && serverPort != 443) || (!useHttps && serverPort != 80))
    {
      urlBuilder << ":" << serverPort;
    }
    
    m_serverUrl = urlBuilder.str();
  }
  else
  {
    m_serverUrl = "";
  }
  
  m_userId = kodi::addon::GetSettingString("user_id", "");
  
  // Try to load access token first (from authentication), fall back to API key
  m_apiKey = kodi::addon::GetSettingString("access_token", "");
  if (m_apiKey.empty())
  {
    m_apiKey = kodi::addon::GetSettingString("api_key", "");
  }

  if (m_serverUrl.empty())
  {
    Logger::Log(ADDON_LOG_ERROR, "Server URL not configured");
    return false;
  }

  Logger::Log(ADDON_LOG_INFO, "Connecting to server: %s", m_serverUrl.c_str());

  // API key/access token is optional at this stage - will be obtained during authentication
  return true;
}

PVR_ERROR CJellyfinPVRClient::GetCapabilities(kodi::addon::PVRCapabilities& capabilities)
{
  capabilities.SetSupportsEPG(true);
  capabilities.SetSupportsTV(true);
  capabilities.SetSupportsRadio(false);
  capabilities.SetSupportsRecordings(true);
  capabilities.SetSupportsTimers(true);
  capabilities.SetSupportsChannelGroups(true);
  capabilities.SetSupportsRecordingsDelete(true);
  capabilities.SetSupportsRecordingsUndelete(false);

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CJellyfinPVRClient::GetBackendName(std::string& name)
{
  name = "Jellyfin Live TV";
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CJellyfinPVRClient::GetBackendVersion(std::string& version)
{
  if (m_jellyfinClient)
  {
    version = m_jellyfinClient->GetServerVersion();
  }
  else
  {
    version = "Unknown";
  }
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CJellyfinPVRClient::GetConnectionString(std::string& connection)
{
  connection = m_serverUrl;
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CJellyfinPVRClient::GetBackendHostname(std::string& hostname)
{
  hostname = m_serverUrl;
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CJellyfinPVRClient::GetChannelsAmount(int& amount)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  amount = m_jellyfinClient->GetChannelCount();
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CJellyfinPVRClient::GetChannels(bool radio, kodi::addon::PVRChannelsResultSet& results)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  // We don't support radio
  if (radio)
    return PVR_ERROR_NO_ERROR;

  return m_jellyfinClient->GetChannels(results);
}

PVR_ERROR CJellyfinPVRClient::GetChannelGroupsAmount(int& amount)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  amount = m_jellyfinClient->GetChannelGroupCount();
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CJellyfinPVRClient::GetChannelGroups(bool radio, kodi::addon::PVRChannelGroupsResultSet& results)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  if (radio)
    return PVR_ERROR_NO_ERROR;

  return m_jellyfinClient->GetChannelGroups(results);
}

PVR_ERROR CJellyfinPVRClient::GetChannelGroupMembers(const kodi::addon::PVRChannelGroup& group,
                                                     kodi::addon::PVRChannelGroupMembersResultSet& results)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  return m_jellyfinClient->GetChannelGroupMembers(group, results);
}

PVR_ERROR CJellyfinPVRClient::GetEPGForChannel(int channelUid,
                                               time_t start,
                                               time_t end,
                                               kodi::addon::PVREPGTagsResultSet& results)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  return m_jellyfinClient->GetEPGForChannel(channelUid, start, end, results);
}

PVR_ERROR CJellyfinPVRClient::GetRecordingsAmount(bool deleted, int& amount)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  amount = m_jellyfinClient->GetRecordingCount(deleted);
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CJellyfinPVRClient::GetRecordings(bool deleted, kodi::addon::PVRRecordingsResultSet& results)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  return m_jellyfinClient->GetRecordings(deleted, results);
}

PVR_ERROR CJellyfinPVRClient::DeleteRecording(const kodi::addon::PVRRecording& recording)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  return m_jellyfinClient->DeleteRecording(recording);
}

PVR_ERROR CJellyfinPVRClient::GetTimersAmount(int& amount)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  amount = m_jellyfinClient->GetTimerCount();
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CJellyfinPVRClient::GetTimers(kodi::addon::PVRTimersResultSet& results)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  return m_jellyfinClient->GetTimers(results);
}

PVR_ERROR CJellyfinPVRClient::AddTimer(const kodi::addon::PVRTimer& timer)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  return m_jellyfinClient->AddTimer(timer);
}

PVR_ERROR CJellyfinPVRClient::DeleteTimer(const kodi::addon::PVRTimer& timer, bool forceDelete)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  return m_jellyfinClient->DeleteTimer(timer);
}

PVR_ERROR CJellyfinPVRClient::GetChannelStreamProperties(const kodi::addon::PVRChannel& channel,
                                                         std::vector<kodi::addon::PVRStreamProperty>& properties)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  return m_jellyfinClient->GetChannelStreamProperties(channel, properties);
}

PVR_ERROR CJellyfinPVRClient::GetRecordingStreamProperties(const kodi::addon::PVRRecording& recording,
                                                           std::vector<kodi::addon::PVRStreamProperty>& properties)
{
  if (!m_jellyfinClient)
    return PVR_ERROR_SERVER_ERROR;

  return m_jellyfinClient->GetRecordingStreamProperties(recording, properties);
}

ADDONCREATOR(CJellyfinAddon)
