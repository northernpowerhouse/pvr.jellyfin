#pragma once

#include <kodi/AddonBase.h>
#include <string>

class Logger
{
public:
  static void Log(const AddonLog level, const char* format, ...);
};
