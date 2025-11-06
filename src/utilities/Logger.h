#pragma once

#include <kodi/AddonBase.h>
#include <string>

class Logger
{
public:
  static void Log(const ADDON_LOG level, const char* format, ...);
};
