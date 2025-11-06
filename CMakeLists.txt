cmake_minimum_required(VERSION 3.5)
project(pvr.jellyfin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if building standalone (e.g., for Android CI)
if(DEFINED KODI_INCLUDE_DIR AND NOT KODI_FOUND)
  message(STATUS "Building in standalone mode with provided KODI_INCLUDE_DIR")
  set(STANDALONE_BUILD TRUE)
  # Manually set up include directories for standalone build
  include_directories(
    ${KODI_INCLUDE_DIR}
    ${KODI_INCLUDE_DIR}/..
  )
else()
  # Normal Kodi build system path
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})
  find_package(Kodi REQUIRED)
  
  # Try to find JsonCpp, but make it optional for now
  find_package(JsonCpp QUIET)
  if(NOT JSONCPP_FOUND)
    message(STATUS "JsonCpp not found via find_package, using fallback")
    set(JSONCPP_INCLUDE_DIRS "")
    set(JSONCPP_LIBRARIES "")
  endif()
  
  include_directories(${KODI_INCLUDE_DIR}/..
                      ${JSONCPP_INCLUDE_DIRS})
endif()

set(JELLYFIN_SOURCES
    src/client.cpp
    src/jellyfin/JellyfinClient.cpp
    src/jellyfin/Connection.cpp
    src/jellyfin/ChannelManager.cpp
    src/jellyfin/EPGManager.cpp
    src/jellyfin/RecordingManager.cpp
    src/utilities/Logger.cpp
    src/utilities/Utilities.cpp)

set(JELLYFIN_HEADERS
    src/client.h
    src/jellyfin/JellyfinClient.h
    src/jellyfin/Connection.h
    src/jellyfin/ChannelManager.h
    src/jellyfin/EPGManager.h
    src/jellyfin/RecordingManager.h
    src/utilities/Logger.h
    src/utilities/Utilities.h)

if(STANDALONE_BUILD)
  # Standalone build - create shared library directly
  add_library(pvr.jellyfin MODULE ${JELLYFIN_SOURCES})
  
  set_target_properties(pvr.jellyfin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "pvr.jellyfin"
    SUFFIX ".so"
  )
  
  # Link against Android log library if on Android
  if(ANDROID)
    target_link_libraries(pvr.jellyfin log)
  endif()
  
  # Install the library
  install(TARGETS pvr.jellyfin
    DESTINATION ${CMAKE_INSTALL_PREFIX}/pvr.jellyfin
  )
  
  # Install addon files
  install(FILES addon.xml
    DESTINATION ${CMAKE_INSTALL_PREFIX}/pvr.jellyfin
  )
  install(DIRECTORY resources
    DESTINATION ${CMAKE_INSTALL_PREFIX}/pvr.jellyfin
  )
  install(FILES LICENSE README.md
    DESTINATION ${CMAKE_INSTALL_PREFIX}/pvr.jellyfin
  )
else()
  # Normal Kodi build
  set(DEPLIBS ${JSONCPP_LIBRARIES})
  build_addon(pvr.jellyfin JELLYFIN DEPLIBS)
endif()

include(CPack)
