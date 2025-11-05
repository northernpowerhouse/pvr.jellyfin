#include "Utilities.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>

namespace Utilities
{

std::string UrlEncode(const std::string& value)
{
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;

  for (char c : value)
  {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
    {
      escaped << c;
    }
    else
    {
      escaped << std::uppercase;
      escaped << '%' << std::setw(2) << int((unsigned char)c);
      escaped << std::nouppercase;
    }
  }

  return escaped.str();
}

std::string Base64Encode(const std::string& input)
{
  static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];
  size_t in_len = input.size();
  const unsigned char* bytes_to_encode = reinterpret_cast<const unsigned char*>(input.c_str());

  while (in_len--)
  {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3)
    {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; i < 4; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

    for (j = 0; j < i + 1; j++)
      ret += base64_chars[char_array_4[j]];

    while (i++ < 3)
      ret += '=';
  }

  return ret;
}

std::vector<std::string> Split(const std::string& str, char delimiter)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(str);
  while (std::getline(tokenStream, token, delimiter))
  {
    tokens.push_back(token);
  }
  return tokens;
}

std::string Join(const std::vector<std::string>& elements, const std::string& delimiter)
{
  std::string result;
  for (size_t i = 0; i < elements.size(); ++i)
  {
    result += elements[i];
    if (i < elements.size() - 1)
      result += delimiter;
  }
  return result;
}

time_t ParseDateTime(const std::string& dateTime)
{
  // Parse ISO 8601 format: 2023-01-01T12:00:00Z
  struct tm tm = {};
  std::istringstream ss(dateTime);
  ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
  
  if (ss.fail())
    return 0;
    
  return timegm(&tm);
}

std::string FormatDateTime(time_t time)
{
  struct tm* tm = gmtime(&time);
  char buffer[80];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", tm);
  return std::string(buffer);
}

} // namespace Utilities
