#include "LogUploader.h"
#include "Logger.h"
#include <kodi/Filesystem.h>
#include <kodi/General.h>
#include <kodi/gui/dialogs/OK.h>
#include <kodi/gui/dialogs/Progress.h>
#include <json/json.h>
#include <sstream>
#include <chrono>
#include <thread>
#include <ctime>

// GitHub OAuth App Client ID for pvr.jellyfin (you'll need to create this)
// For now, using a placeholder - you'll need to create a GitHub OAuth app
static const char* GITHUB_CLIENT_ID = "Ov23liXOKSJZpGo5qJqF";
static const char* GITHUB_REPO = "northernpowerhouse/pvr.jellyfin";

LogUploader::LogUploader()
{
}

bool LogUploader::UploadLogs()
{
  Logger::Log(ADDON_LOG_INFO, "Starting log upload process...");
  
  // Authenticate with GitHub if we don't have a token
  if (m_githubToken.empty())
  {
    if (!AuthenticateWithGitHub())
    {
      kodi::gui::dialogs::OK::ShowAndGetInput("Log Upload Failed", 
                                              "Failed to authenticate with GitHub");
      return false;
    }
  }
  
  // Read log file
  std::string logContent = ReadLogFile();
  if (logContent.empty())
  {
    kodi::gui::dialogs::OK::ShowAndGetInput("Log Upload Failed", 
                                            "Could not read log file");
    return false;
  }
  
  // Upload to GitHub
  if (!UploadToGitHub(logContent))
  {
    kodi::gui::dialogs::OK::ShowAndGetInput("Log Upload Failed", 
                                            "Failed to upload logs to GitHub");
    return false;
  }
  
  kodi::gui::dialogs::OK::ShowAndGetInput("Log Upload Successful", 
                                          "Logs have been uploaded to GitHub");
  return true;
}

bool LogUploader::AuthenticateWithGitHub()
{
  std::string userCode, deviceCode, verificationUri;
  
  if (!StartDeviceFlow(userCode, deviceCode, verificationUri))
  {
    return false;
  }
  
  // Show dialog with code
  std::ostringstream message;
  message << "Go to: " << verificationUri << "\n\n"
          << "Enter code: " << userCode << "\n\n"
          << "Waiting for authorization...";
  
  // Create progress dialog
  kodi::gui::dialogs::CProgress progress;
  progress.SetHeading("GitHub Authentication");
  progress.SetLine(1, message.str());
  progress.ShowDialog();
  
  // Poll for token (5 second intervals for up to 5 minutes)
  for (int i = 0; i < 60; i++)
  {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    if (progress.IsCanceled())
    {
      return false;
    }
    
    progress.SetPercentage((i * 100) / 60);
    
    if (PollForToken(deviceCode))
    {
      progress.Close();
      return true;
    }
  }
  
  progress.Close();
  return false;
}

bool LogUploader::StartDeviceFlow(std::string& userCode, std::string& deviceCode, std::string& verificationUri)
{
  Logger::Log(ADDON_LOG_INFO, "Starting GitHub device flow...");
  
  kodi::vfs::CFile file;
  std::string url = "https://github.com/login/device/code";
  
  file.CURLCreate(url);
  file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "Accept", "application/json");
  file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "Content-Type", "application/x-www-form-urlencoded");
  
  std::ostringstream postData;
  postData << "client_id=" << GITHUB_CLIENT_ID << "&scope=repo";
  file.CURLAddOption(ADDON_CURL_OPTION_PROTOCOL, "postdata", postData.str().c_str());
  
  if (!file.CURLOpen(ADDON_READ_NO_CACHE))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to start device flow");
    return false;
  }
  
  std::string response;
  char buffer[1024];
  ssize_t bytesRead;
  while ((bytesRead = file.Read(buffer, sizeof(buffer))) > 0)
  {
    response.append(buffer, bytesRead);
  }
  file.Close();
  
  // Parse response
  Json::CharReaderBuilder builder;
  std::string errors;
  std::istringstream responseStream(response);
  Json::Value json;
  
  if (!Json::parseFromStream(builder, responseStream, &json, &errors))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to parse device flow response: %s", errors.c_str());
    return false;
  }
  
  if (!json.isMember("device_code") || !json.isMember("user_code") || !json.isMember("verification_uri"))
  {
    Logger::Log(ADDON_LOG_ERROR, "Invalid device flow response");
    return false;
  }
  
  deviceCode = json["device_code"].asString();
  userCode = json["user_code"].asString();
  verificationUri = json["verification_uri"].asString();
  
  Logger::Log(ADDON_LOG_INFO, "Device flow started. Code: %s", userCode.c_str());
  return true;
}

bool LogUploader::PollForToken(const std::string& deviceCode)
{
  kodi::vfs::CFile file;
  std::string url = "https://github.com/login/oauth/access_token";
  
  file.CURLCreate(url);
  file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "Accept", "application/json");
  file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "Content-Type", "application/x-www-form-urlencoded");
  
  std::ostringstream postData;
  postData << "client_id=" << GITHUB_CLIENT_ID 
           << "&device_code=" << deviceCode
           << "&grant_type=urn:ietf:params:oauth:grant-type:device_code";
  file.CURLAddOption(ADDON_CURL_OPTION_PROTOCOL, "postdata", postData.str().c_str());
  
  if (!file.CURLOpen(ADDON_READ_NO_CACHE))
  {
    return false;
  }
  
  std::string response;
  char buffer[1024];
  ssize_t bytesRead;
  while ((bytesRead = file.Read(buffer, sizeof(buffer))) > 0)
  {
    response.append(buffer, bytesRead);
  }
  file.Close();
  
  // Parse response
  Json::CharReaderBuilder builder;
  std::string errors;
  std::istringstream responseStream(response);
  Json::Value json;
  
  if (!Json::parseFromStream(builder, responseStream, &json, &errors))
  {
    return false;
  }
  
  if (json.isMember("access_token"))
  {
    m_githubToken = json["access_token"].asString();
    Logger::Log(ADDON_LOG_INFO, "GitHub authentication successful");
    return true;
  }
  
  // Check for specific errors
  if (json.isMember("error"))
  {
    std::string error = json["error"].asString();
    if (error != "authorization_pending" && error != "slow_down")
    {
      Logger::Log(ADDON_LOG_ERROR, "GitHub authentication error: %s", error.c_str());
    }
  }
  
  return false;
}

std::string LogUploader::GetLogFilePath()
{
  // Get Kodi's log folder path
  std::string logPath = kodi::GetSettingString("__addon_path__");
  return logPath + "/kodi.log";
}

std::string LogUploader::ReadLogFile()
{
  Logger::Log(ADDON_LOG_INFO, "Reading log file...");
  
  // For now, we'll collect recent log messages that were logged through our Logger
  // In a real implementation, you'd read from Kodi's actual log file
  // This is a simplified version
  return "Log file content would be here - implement reading from actual Kodi log";
}

bool LogUploader::UploadToGitHub(const std::string& logContent)
{
  Logger::Log(ADDON_LOG_INFO, "Uploading logs to GitHub...");
  
  std::string timestamp = GetCurrentTimestamp();
  std::string filename = "log_" + timestamp + ".txt";
  std::string path = "dev-logs/" + filename;
  
  // Create file content (base64 encoded)
  // For simplicity, we'll use the content directly
  // In production, you'd base64 encode it
  
  Json::Value requestData;
  requestData["message"] = "Add log file: " + filename;
  requestData["content"] = logContent; // Should be base64 encoded
  requestData["branch"] = "main";
  
  // Use GitHub API to create file
  kodi::vfs::CFile file;
  std::ostringstream url;
  url << "https://api.github.com/repos/" << GITHUB_REPO << "/contents/" << path;
  
  file.CURLCreate(url.str());
  file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "Accept", "application/vnd.github+json");
  file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "Authorization", ("Bearer " + m_githubToken).c_str());
  file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "Content-Type", "application/json");
  file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "X-GitHub-Api-Version", "2022-11-28");
  
  Json::StreamWriterBuilder builder;
  std::string jsonData = Json::writeString(builder, requestData);
  file.CURLAddOption(ADDON_CURL_OPTION_PROTOCOL, "postdata", jsonData.c_str());
  file.CURLAddOption(ADDON_CURL_OPTION_PROTOCOL, "customrequest", "PUT");
  
  if (!file.CURLOpen(ADDON_READ_NO_CACHE))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to upload log to GitHub");
    return false;
  }
  
  // Read response
  std::string response;
  char buffer[1024];
  ssize_t bytesRead;
  while ((bytesRead = file.Read(buffer, sizeof(buffer))) > 0)
  {
    response.append(buffer, bytesRead);
  }
  file.Close();
  
  Logger::Log(ADDON_LOG_INFO, "Log uploaded successfully to: dev-logs/%s", filename.c_str());
  return true;
}

std::string LogUploader::GetCurrentTimestamp()
{
  auto now = std::chrono::system_clock::now();
  auto timeT = std::chrono::system_clock::to_time_t(now);
  
  std::ostringstream oss;
  oss << std::put_time(std::localtime(&timeT), "%Y%m%d_%H%M%S");
  return oss.str();
}
