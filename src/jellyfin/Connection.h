#pragma once

#include <string>
#include <json/json.h>

class Connection
{
public:
  Connection(const std::string& serverUrl, const std::string& apiKey);
  ~Connection() = default;

  bool SendRequest(const std::string& endpoint, Json::Value& response);
  bool SendPostRequest(const std::string& endpoint, const Json::Value& data, Json::Value& response);
  bool SendDeleteRequest(const std::string& endpoint);
  
  std::string GetServerUrl() const { return m_serverUrl; }
  std::string GetApiKey() const { return m_apiKey; }

private:
  std::string m_serverUrl;
  std::string m_apiKey;
  
  std::string BuildUrl(const std::string& endpoint) const;
  std::string PerformHttpGet(const std::string& url);
  std::string PerformHttpPost(const std::string& url, const std::string& data);
  bool PerformHttpDelete(const std::string& url);
};
