#include "AuthManager.h"
#include "Connection.h"
#include "../utilities/Logger.h"
#include <sstream>

AuthManager::AuthManager(Connection* connection)
  : m_connection(connection)
{
}

bool AuthManager::AuthenticateByPassword(const std::string& username, const std::string& password,
                                         std::string& userId, std::string& accessToken)
{
  Logger::Log(ADDON_LOG_INFO, "Attempting password authentication for user: %s", username.c_str());
  
  // Jellyfin API: Username (capitalized) and Pw (not Password)
  // Reference: https://api.jellyfin.org/#tag/User/operation/AuthenticateUserByName
  // CRITICAL: jsoncpp sorts keys alphabetically, but Jellyfin expects Username before Pw
  // Build JSON manually to guarantee exact order that works in test script
  Json::Value requestData;
  requestData["Username"] = username;
  requestData["Pw"] = password;
  
  Logger::Log(ADDON_LOG_INFO, "Auth request - username: %s, Endpoint: /Users/AuthenticateByName", username.c_str());
  
  Json::Value response;
  if (!m_connection->SendPostRequest("/Users/AuthenticateByName", requestData, response))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to authenticate user: %s", username.c_str());
    return false;
  }
  
  if (!response.isMember("AccessToken") || !response.isMember("User"))
  {
    Logger::Log(ADDON_LOG_ERROR, "Invalid authentication response");
    return false;
  }
  
  accessToken = response["AccessToken"].asString();
  userId = response["User"]["Id"].asString();
  
  Logger::Log(ADDON_LOG_INFO, "Successfully authenticated user: %s", username.c_str());
  return true;
}

bool AuthManager::StartQuickConnect(std::string& code)
{
  Logger::Log(ADDON_LOG_INFO, "Starting Quick Connect...");
  
  Json::Value response;
  if (!m_connection->SendRequest("/QuickConnect/Initiate", response))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to initiate Quick Connect");
    return false;
  }
  
  if (!response.isMember("Code") || !response.isMember("Secret"))
  {
    Logger::Log(ADDON_LOG_ERROR, "Invalid Quick Connect response");
    return false;
  }
  
  code = response["Code"].asString();
  m_quickConnectSecret = response["Secret"].asString();
  
  Logger::Log(ADDON_LOG_INFO, "Quick Connect initiated with code: %s", code.c_str());
  return true;
}

bool AuthManager::CheckQuickConnectStatus(std::string& userId, std::string& accessToken)
{
  if (m_quickConnectSecret.empty())
  {
    Logger::Log(ADDON_LOG_ERROR, "Quick Connect secret not initialized");
    return false;
  }
  
  std::ostringstream endpoint;
  endpoint << "/QuickConnect/Connect?secret=" << m_quickConnectSecret;
  
  Json::Value response;
  if (!m_connection->SendRequest(endpoint.str(), response))
  {
    return false;
  }
  
  // Check if authenticated
  if (!response.isMember("Authenticated") || !response["Authenticated"].asBool())
  {
    return false;
  }
  
  if (!response.isMember("Authentication"))
  {
    Logger::Log(ADDON_LOG_ERROR, "Quick Connect authenticated but missing token");
    return false;
  }
  
  const Json::Value& auth = response["Authentication"];
  if (!auth.isMember("AccessToken") || !auth.isMember("UserId"))
  {
    Logger::Log(ADDON_LOG_ERROR, "Invalid Quick Connect authentication data");
    return false;
  }
  
  accessToken = auth["AccessToken"].asString();
  userId = auth["UserId"].asString();
  
  Logger::Log(ADDON_LOG_INFO, "Quick Connect authentication successful");
  return true;
}

bool AuthManager::ValidateToken(const std::string& userId, const std::string& token)
{
  Logger::Log(ADDON_LOG_INFO, "Validating access token for user: %s", userId.c_str());
  
  std::ostringstream endpoint;
  endpoint << "/Users/" << userId;
  
  Json::Value response;
  if (!m_connection->SendRequest(endpoint.str(), response))
  {
    Logger::Log(ADDON_LOG_ERROR, "Failed to validate token");
    return false;
  }
  
  if (!response.isMember("Id"))
  {
    Logger::Log(ADDON_LOG_ERROR, "Invalid validation response");
    return false;
  }
  
  Logger::Log(ADDON_LOG_INFO, "Token is valid");
  return true;
}
