#include "Connection.h"
#include "../utilities/Logger.h"
#include <kodi/Network.h>
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
  
  // Add API key parameter
  if (endpoint.find('?') != std::string::npos)
  {
    url << "&api_key=" << m_apiKey;
  }
  else
  {
    url << "?api_key=" << m_apiKey;
  }
  
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
  
  Json::StreamWriterBuilder builder;
  std::string jsonData = Json::writeString(builder, data);
  
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
  kodi::network::CURLRequest request;
  request.SetHeader("Accept", "application/json");
  request.SetHeader("X-Emby-Authorization", "MediaBrowser Client=\"Kodi\", Device=\"Kodi\", DeviceId=\"kodi-pvr-jellyfin\", Version=\"1.0.0\"");
  
  std::string response;
  if (!request.Get(url, response))
  {
    Logger::Log(ADDON_LOG_ERROR, "HTTP GET failed for URL: %s", url.c_str());
    return "";
  }
  
  return response;
}

std::string Connection::PerformHttpPost(const std::string& url, const std::string& data)
{
  kodi::network::CURLRequest request;
  request.SetHeader("Content-Type", "application/json");
  request.SetHeader("Accept", "application/json");
  request.SetHeader("X-Emby-Authorization", "MediaBrowser Client=\"Kodi\", Device=\"Kodi\", DeviceId=\"kodi-pvr-jellyfin\", Version=\"1.0.0\"");
  
  std::string response;
  if (!request.Post(url, data, response))
  {
    Logger::Log(ADDON_LOG_ERROR, "HTTP POST failed for URL: %s", url.c_str());
    return "";
  }
  
  return response;
}

bool Connection::PerformHttpDelete(const std::string& url)
{
  kodi::network::CURLRequest request;
  request.SetHeader("X-Emby-Authorization", "MediaBrowser Client=\"Kodi\", Device=\"Kodi\", DeviceId=\"kodi-pvr-jellyfin\", Version=\"1.0.0\"");
  
  std::string response;
  if (!request.Delete(url, response))
  {
    Logger::Log(ADDON_LOG_ERROR, "HTTP DELETE failed for URL: %s", url.c_str());
    return false;
  }
  
  return true;
}
