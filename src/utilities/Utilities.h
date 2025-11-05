#pragma once

#include <string>
#include <vector>

namespace Utilities
{
  std::string UrlEncode(const std::string& value);
  std::string Base64Encode(const std::string& input);
  std::vector<std::string> Split(const std::string& str, char delimiter);
  std::string Join(const std::vector<std::string>& elements, const std::string& delimiter);
  time_t ParseDateTime(const std::string& dateTime);
  std::string FormatDateTime(time_t time);
}
