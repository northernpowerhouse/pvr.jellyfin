#pragma once

#include <string>
#include <json/json.h>

class Connection;

class AuthManager
{
public:
  AuthManager(Connection* connection);
  ~AuthManager() = default;

  // Authenticate with username and password
  bool AuthenticateByPassword(const std::string& username, const std::string& password, 
                              std::string& userId, std::string& accessToken);
  
  // Quick Connect authentication
  bool StartQuickConnect(std::string& code);
  bool CheckQuickConnectStatus(const std::string& secret, std::string& userId, std::string& accessToken);
  
  // Validate existing token
  bool ValidateToken(const std::string& userId, const std::string& token);

private:
  Connection* m_connection;
  
  std::string m_quickConnectSecret;
};
