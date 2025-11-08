cmake_minimum_required(VERSION 3.5)
project(pvr.jellyfin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Override version with our VERSION file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" ADDON_VERSION)
string(STRIP "${ADDON_VERSION}" ADDON_VERSION)
message(STATUS "Using version from VERSION file: ${ADDON_VERSION}")

# Check if building standalone (e.g., for Android CI)
if(DEFINED KODI_INCLUDE_DIR AND NOT KODI_FOUND)
  message(STATUS "Building in standalone mode with provided KODI_INCLUDE_DIR: ${KODI_INCLUDE_DIR}")
  set(STANDALONE_BUILD TRUE)
  
  # Try to find JsonCpp for standalone build
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(JSONCPP jsoncpp)
    message(STATUS "JsonCpp found via pkg-config: ${JSONCPP_INCLUDE_DIRS}")
  endif()
  
  if(NOT JSONCPP_FOUND)
    # Try to find jsoncpp manually in common locations
    # jsoncpp is often bundled with Kodi in the CMake utilities
    find_path(JSONCPP_INCLUDE_DIR json/json.h
      HINTS
        ${CMAKE_PREFIX_PATH}/include
        /opt/xbmc-depends/arm-linux-androideabi-21-release/include
        /opt/kodi/tools/depends/native/cmake/x86_64-linux-native/Utilities/cmjsoncpp/include
        /usr/include
        /usr/local/include
    )
    if(JSONCPP_INCLUDE_DIR)
      set(JSONCPP_INCLUDE_DIRS ${JSONCPP_INCLUDE_DIR})
      message(STATUS "JsonCpp found manually: ${JSONCPP_INCLUDE_DIRS}")
    endif()
  endif()
  
  # Manually set up include directories for standalone build
  include_directories(
    ${KODI_INCLUDE_DIR}
    ${KODI_INCLUDE_DIR}/..
    ${JSONCPP_INCLUDE_DIRS}
    ${CMAKE_PREFIX_PATH}/include
  )
  
  message(STATUS "Include directories: ${KODI_INCLUDE_DIR}, ${JSONCPP_INCLUDE_DIRS}")
  
  # Add system include for standard libraries
  if(ANDROID)
    include_directories(SYSTEM /usr/include)
  endif()
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
    src/jellyfin/AuthManager.cpp
    src/utilities/Logger.cpp
    src/utilities/Utilities.cpp)

set(JELLYFIN_HEADERS
    src/client.h
    src/jellyfin/JellyfinClient.h
    src/jellyfin/Connection.h
    src/jellyfin/ChannelManager.h
    src/jellyfin/EPGManager.h
    src/jellyfin/RecordingManager.h
    src/jellyfin/AuthManager.h
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
  
  # Find jsoncpp library - built in Docker image at /opt/jsoncpp-android
  set(JSONCPP_LIB_PATH "/opt/jsoncpp-android/lib/libjsoncpp.a")
  set(JSONCPP_INCLUDE_DIR "/opt/jsoncpp-android/include")
  
  # Check if library exists
  if(EXISTS ${JSONCPP_LIB_PATH})
    message(STATUS "jsoncpp library found at: ${JSONCPP_LIB_PATH}")
    target_include_directories(pvr.jellyfin PRIVATE ${JSONCPP_INCLUDE_DIR})
    target_link_libraries(pvr.jellyfin ${JSONCPP_LIB_PATH})
  else()
    message(FATAL_ERROR "jsoncpp library not found at ${JSONCPP_LIB_PATH}")
  endif()
  
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
