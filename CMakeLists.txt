cmake_minimum_required(VERSION 3.5)
project(pvr.jellyfin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(Kodi REQUIRED)
find_package(JsonCpp REQUIRED)

include_directories(${KODI_INCLUDE_DIR}/..
                    ${JSONCPP_INCLUDE_DIRS})

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

set(DEPLIBS ${JSONCPP_LIBRARIES})

build_addon(pvr.jellyfin JELLYFIN DEPLIBS)

include(CPack)
