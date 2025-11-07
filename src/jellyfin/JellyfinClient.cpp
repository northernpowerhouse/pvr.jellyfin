#include "JellyfinClient.h"
#include "Connection.h"
#include "ChannelManager.h"
#include "EPGManager.h"
#include "RecordingManager.h"
#include "AuthManager.h"
#include "../utilities/Logger.h"
#include <json/json.h>
#include <kodi/gui/dialogs/OK.h>
#include <kodi/gui/dialogs/Progress.h>
#include <thread>
#include <chrono>

JellyfinClient::JellyfinClient(const std::string& serverUrl, const std::string& userId, const std::string& apiKey)
  : m_serverUrl(serverUrl)
  , m_userId(userId)
  , m_apiKey(apiKey)
  , m_serverVersion("Unknown")
  , m_authenticated(false)
{
  m_connection = std::make_unique<Connection>(serverUrl, apiKey);
  m_authManager = std::make_unique<AuthManager>(m_connection.get());
}

JellyfinClient::~JellyfinClient() = default;

bool JellyfinClient::Initialize()
{
  Logger::Log(ADDON_LOG_INFO, "Initializing Jellyfin client...");
  
  // If we have a userId and apiKey, try to validate them
  if (!m_userId.empty() && !m_apiKey.empty())
  {
    if (m_authManager->ValidateToken(m_userId, m_apiKey))
    {
      Logger::Log(ADDON_LOG_INFO, "Existing credentials are valid");
      m_authenticated = true;
      return Connect();
    }
    else
    {
      Logger::Log(ADDON_LOG_WARNING, "Existing credentials are invalid, need to re-authenticate");
    }
  }
  
  return false;
}

bool JellyfinClient::AuthenticateWithPassword(const std::string& username, const std::string& password)
{
  Logger::Log(ADDON_LOG_INFO, "Authenticating with username and password...");
  
  std::string userId, accessToken;
  if (!m_authManager->AuthenticateByPassword(username, password, userId, accessToken))
  {
    kodi::gui::dialogs::OK::ShowAndGetInput("Authentication Failed", 
                                            "Could not authenticate with Jellyfin.\nPlease check your username and password.");
    return false;
  }
  
  // Update credentials
  m_userId = userId;
  m_apiKey = accessToken;
  m_authenticated = true;
  
  // Save to settings
  kodi::addon::SetSettingString("user_id", m_userId);
  kodi::addon::SetSettingString("access_token", m_apiKey);
  
  Logger::Log(ADDON_LOG_INFO, "Authentication successful, user ID: %s", m_userId.c_str());
  
  // Reconnect with new credentials
  m_connection = std::make_unique<Connection>(m_serverUrl, m_apiKey);
  m_authManager = std::make_unique<AuthManager>(m_connection.get());
  
  return Connect();
}

bool JellyfinClient::AuthenticateWithQuickConnect()
{
  Logger::Log(ADDON_LOG_INFO, "Starting Quick Connect authentication...");
  
  std::string code;
  if (!m_authManager->StartQuickConnect(code))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to start Quick Connect");
    kodi::gui::dialogs::OK::ShowAndGetInput("Quick Connect Failed", 
                                            "Could not start Quick Connect.\nPlease try again.");
    return false;
  }
  
  // Log the code prominently (not debug level, use INFO)
  Logger::Log(ADDON_LOG_INFO, "========================================");
  Logger::Log(ADDON_LOG_INFO, "QUICK CONNECT CODE: %s", code.c_str());
  Logger::Log(ADDON_LOG_INFO, "========================================");
  
  // First show the code in a prominent OK dialog
  Logger::Log(ADDON_LOG_INFO, "Attempting to show Quick Connect dialog...");
  
  std::ostringstream codeMessage;
  codeMessage << "Your Quick Connect code is:\n\n"
              << "[B][COLOR yellow]" << code << "[/COLOR][/B]\n\n"
              << "Go to Jellyfin Dashboard > Quick Connect\n"
              << "and enter this code.\n\n"
              << "Click OK to continue waiting for authorization...";
  
  kodi::gui::dialogs::OK::ShowAndGetInput("Quick Connect Code", codeMessage.str());
  
  Logger::Log(ADDON_LOG_INFO, "Quick Connect dialog shown, waiting for authorization...");
  
  // Show progress dialog while waiting
  kodi::gui::dialogs::CProgress* progress = new kodi::gui::dialogs::CProgress();
  progress->SetHeading("Quick Connect - Waiting...");
  progress->SetLine(1, "Waiting for you to authorize on Jellyfin...");
  progress->SetLine(2, "Code: " + code);
  
  // Poll for authentication (every 3 seconds for up to 5 minutes)
  std::string userId, accessToken;
  for (int i = 0; i < 100; i++)
  {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    if (progress->IsCanceled())
    {
      Logger::Log(ADDON_LOG_INFO, "Quick Connect cancelled by user");
      delete progress;
      return false;
    }
    
    progress->SetPercentage((i * 100) / 100);
    
    if (m_authManager->CheckQuickConnectStatus(userId, accessToken))
    {
      delete progress;
      
      // Update credentials
      m_userId = userId;
      m_apiKey = accessToken;
      m_authenticated = true;
      
      // Save to settings
      kodi::addon::SetSettingString("user_id", m_userId);
      kodi::addon::SetSettingString("access_token", m_apiKey);
      
      Logger::Log(ADDON_LOG_INFO, "Quick Connect successful, user ID: %s", m_userId.c_str());
      
      // Reconnect with new credentials
      m_connection = std::make_unique<Connection>(m_serverUrl, m_apiKey);
      m_authManager = std::make_unique<AuthManager>(m_connection.get());
      
      kodi::gui::dialogs::OK::ShowAndGetInput("Quick Connect Successful", 
                                              "You are now connected to Jellyfin!");
      
      return Connect();
    }
  }
  
  delete progress;
  kodi::gui::dialogs::OK::ShowAndGetInput("Quick Connect Timeout", 
                                          "Quick Connect timed out.\nPlease try again.");
  return false;
}

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
  
  // When using API key authentication, user ID must be provided in settings
  // API keys cannot access /Users/Me endpoint, so we require manual configuration
  if (!m_apiKey.empty() && m_userId.empty())
  {
    Logger::Log(ADDON_LOG_ERROR, "User ID is required when using API key authentication. Please configure it in addon settings.");
    return false;
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
  if (m_epgManager && m_channelManager)
  {
    // Get Jellyfin channel ID from UID
    std::string jellyfinChannelId = m_channelManager->GetChannelIdFromUid(channelUid);
    if (jellyfinChannelId.empty())
    {
      Logger::Log(ADDON_LOG_WARNING, "Could not find Jellyfin channel ID for UID: %d", channelUid);
      return PVR_ERROR_NO_ERROR; // Return success but with no entries
    }
    
    return m_epgManager->GetEPGForChannel(channelUid, start, end, results, jellyfinChannelId);
  }
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
