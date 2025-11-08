#include "Connection.h"
#include "../utilities/Logger.h"
#include <kodi/Filesystem.h>
#include <sstream>

Connection::Connection(const std::string& serverUrl, const std::string& apiKey)
  : m_serverUrl(serverUrl)
  , m_apiKey(apiKey)
{
  // Remove trailing slash from server URL if present
  if (!m_serverUrl.empty() && m_serverUrl.back() == '/')
  {
    m_serverUrl.pop_back();
  }
}

std::string Connection::BuildUrl(const std::string& endpoint) const
{
  std::ostringstream url;
  url << m_serverUrl << endpoint;
  
  // For Jellyfin 10.10+, we send the token via header instead of query param
  // This is handled in the HTTP methods
  
  return url.str();
}

bool Connection::SendRequest(const std::string& endpoint, Json::Value& response)
{
  std::string url = BuildUrl(endpoint);
  std::string responseStr = PerformHttpGet(url);
  
  if (responseStr.empty())
  {
    Logger::Log(ADDON_LOG_ERROR, "Empty response from server for endpoint: %s", endpoint.c_str());
    return false;
  }

  Json::CharReaderBuilder builder;
  std::string errors;
  std::istringstream responseStream(responseStr);
  
  if (!Json::parseFromStream(builder, responseStream, &response, &errors))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to parse JSON response: %s", errors.c_str());
    return false;
  }

  return true;
}

bool Connection::SendPostRequest(const std::string& endpoint, const Json::Value& data, Json::Value& response)
{
  std::string url = BuildUrl(endpoint);
  
  // Use compact JSON (no indentation) for POST requests
  Json::StreamWriterBuilder builder;
  builder["indentation"] = "";
  std::string jsonData = Json::writeString(builder, data);
  
  // Remove any trailing newline that jsoncpp might add
  if (!jsonData.empty() && jsonData.back() == '\n')
  {
    jsonData.pop_back();
  }
  
  std::string responseStr = PerformHttpPost(url, jsonData);
  
  if (responseStr.empty())
  {
    Logger::Log(ADDON_LOG_ERROR, "Empty response from server for endpoint: %s", endpoint.c_str());
    return false;
  }

  Json::CharReaderBuilder readerBuilder;
  std::string errors;
  std::istringstream responseStream(responseStr);
  
  if (!Json::parseFromStream(readerBuilder, responseStream, &response, &errors))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to parse JSON response: %s", errors.c_str());
    return false;
  }

  return true;
}

bool Connection::SendDeleteRequest(const std::string& endpoint)
{
  std::string url = BuildUrl(endpoint);
  return PerformHttpDelete(url);
}

std::string Connection::PerformHttpGet(const std::string& url)
{
  kodi::vfs::CFile file;
  file.CURLCreate(url);
  file.CURLAddOption(ADDON_CURL_OPTION_PROTOCOL, "acceptencoding", "gzip");
  file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "Accept", "application/json");
  
  // Jellyfin 10.10+ compatible authentication header
  std::ostringstream authHeader;
  authHeader << "MediaBrowser Client=\"Kodi PVR\", Device=\"Kodi\", DeviceId=\"kodi-pvr-jellyfin\", Version=\"1.0.0\", Token=\"" << m_apiKey << "\"";
  
  // Log auth header for debugging (with token preview)
  std::string headerStr = authHeader.str();
  std::string tokenPreview = "none";
  if (!m_apiKey.empty())
  {
    if (m_apiKey.length() > 8)
    {
      tokenPreview = m_apiKey.substr(0, 4) + "..." + m_apiKey.substr(m_apiKey.length() - 4);
    }
    else
    {
      tokenPreview = "****";
    }
  }
  Logger::Log(ADDON_LOG_DEBUG, "HTTP GET %s with Token: %s", url.c_str(), tokenPreview.c_str());
  
  file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "X-Emby-Authorization", authHeader.str().c_str());
  
  if (!file.CURLOpen(ADDON_READ_NO_CACHE))
  {
    Logger::Log(ADDON_LOG_ERROR, "HTTP GET failed for URL: %s", url.c_str());
    return "";
  }
  
  std::string response;
  char buffer[1024];
  ssize_t bytesRead;
  while ((bytesRead = file.Read(buffer, sizeof(buffer))) > 0)
  {
    response.append(buffer, bytesRead);
  }
  file.Close();
  
  return response;
}

std::string Connection::PerformHttpPost(const std::string& url, const std::string& data)
{
  Logger::Log(ADDON_LOG_DEBUG, "HTTP POST to: %s", url.c_str());
  Logger::Log(ADDON_LOG_DEBUG, "POST data (%zu bytes): %s", data.length(), data.c_str());
  
  kodi::vfs::CFile file;
  file.CURLCreate(url);
  file.CURLAddOption(ADDON_CURL_OPTION_PROTOCOL, "acceptencoding", "gzip");
  file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "Content-Type", "application/json");
  file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "Accept", "application/json");
  
  // Only add authentication header if we have a token
  if (!m_apiKey.empty())
  {
    std::ostringstream authHeader;
    authHeader << "MediaBrowser Client=\"Kodi PVR\", Device=\"Kodi\", DeviceId=\"kodi-pvr-jellyfin\", Version=\"1.0.0\", Token=\"" << m_apiKey << "\"";
    file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "X-Emby-Authorization", authHeader.str().c_str());
    Logger::Log(ADDON_LOG_DEBUG, "Auth header (with token)");
  }
  else
  {
    // For unauthenticated requests (like login), still need the client identification
    file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "X-Emby-Authorization", "MediaBrowser Client=\"Kodi PVR\", Device=\"Kodi\", DeviceId=\"kodi-pvr-jellyfin\", Version=\"1.0.0\"");
    Logger::Log(ADDON_LOG_DEBUG, "Auth header (no token)");
  }
  
  // Let CURL calculate Content-Length automatically from postdata
  file.CURLAddOption(ADDON_CURL_OPTION_PROTOCOL, "postdata", data.c_str());
  
  bool openSuccess = file.CURLOpen(ADDON_READ_NO_CACHE);
  
  // Try to read response regardless of openSuccess, as error responses may still have body
  std::string response;
  char buffer[1024];
  ssize_t bytesRead;
  while ((bytesRead = file.Read(buffer, sizeof(buffer))) > 0)
  {
    response.append(buffer, bytesRead);
  }
  file.Close();
  
  if (!openSuccess)
  {
    Logger::Log(ADDON_LOG_ERROR, "HTTP POST failed for URL: %s", url.c_str());
    Logger::Log(ADDON_LOG_ERROR, "POST request body was: %s", data.c_str());
    Logger::Log(ADDON_LOG_ERROR, "X-Emby-Authorization header: %s", 
                m_apiKey.empty() ? "MediaBrowser Client=\"Kodi PVR\", Device=\"Kodi\", DeviceId=\"kodi-pvr-jellyfin\", Version=\"1.0.0\"" : "(with token)");
    if (!response.empty())
    {
      Logger::Log(ADDON_LOG_ERROR, "HTTP error response body: %s", response.c_str());
    }
    else
    {
      Logger::Log(ADDON_LOG_ERROR, "No response body available");
    }
    return "";
  }
  
  Logger::Log(ADDON_LOG_DEBUG, "HTTP POST response (%zu bytes): %.500s", response.length(), response.c_str());
  
  return response;
}

bool Connection::PerformHttpDelete(const std::string& url)
{
  kodi::vfs::CFile file;
  file.CURLCreate(url);
  file.CURLAddOption(ADDON_CURL_OPTION_PROTOCOL, "customrequest", "DELETE");
  
  // Jellyfin 10.10+ compatible authentication header
  std::ostringstream authHeader;
  authHeader << "MediaBrowser Client=\"Kodi PVR\", Device=\"Kodi\", DeviceId=\"kodi-pvr-jellyfin\", Version=\"1.0.0\", Token=\"" << m_apiKey << "\"";
  file.CURLAddOption(ADDON_CURL_OPTION_HEADER, "X-Emby-Authorization", authHeader.str().c_str());
  
  if (!file.CURLOpen(ADDON_READ_NO_CACHE))
  {
    Logger::Log(ADDON_LOG_ERROR, "HTTP DELETE failed for URL: %s", url.c_str());
    return false;
  }
  
  return true;
}
