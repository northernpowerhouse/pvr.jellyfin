#pragma once

#include <string>

class LogUploader
{
public:
  LogUploader();
  ~LogUploader() = default;

  // Upload logs to GitHub using device flow authentication
  bool UploadLogs();

private:
  std::string m_githubToken;
  
  // GitHub Device Flow authentication
  bool AuthenticateWithGitHub();
  bool StartDeviceFlow(std::string& userCode, std::string& deviceCode, std::string& verificationUri);
  bool PollForToken(const std::string& deviceCode);
  
  // File operations
  std::string GetLogFilePath();
  std::string ReadLogFile();
  std::string FilterAddonLogs(const std::string& fullLog);
  std::string CollectAddonLogs();
  
  // GitHub API
  bool UploadToGitHub(const std::string& logContent);
  std::string GetCurrentTimestamp();
};
