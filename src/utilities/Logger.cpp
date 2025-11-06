#include "Logger.h"
#include <kodi/AddonBase.h>
#include <cstdarg>
#include <cstdio>

void Logger::Log(const ADDON_LOG level, const char* format, ...)
{
  char buffer[16384];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  
  kodi::Log(level, "%s", buffer);
}
