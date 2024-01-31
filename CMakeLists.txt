cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
include(FetchContent)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

project(DCCEXProtocol LANGUAGES CXX)

file(GLOB_RECURSE SRC src/*.cpp)
add_library(DCCEXProtocol STATIC ${SRC})
add_library(DCCEX::Protocol ALIAS DCCEXProtocol)

# Requires at least C++11
target_compile_features(DCCEXProtocol PUBLIC cxx_std_11)

# Don't bother users with warnings by setting 'SYSTEM'
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  target_include_directories(DCCEXProtocol PUBLIC src)
else()
  target_include_directories(DCCEXProtocol SYSTEM PUBLIC src)
endif()

# Fetch ArduinoCore-API
if(NOT TARGET ArduinoCore-API)
  FetchContent_Declare(
    ArduinoCore-API
    GIT_REPOSITORY https://github.com/arduino/ArduinoCore-API
    GIT_TAG 1.4.0)
  FetchContent_Populate(ArduinoCore-API)
  find_package(ArduinoCore-API)
endif()

target_link_libraries(DCCEXProtocol PUBLIC ArduinoCore-API)

foreach(FILE ${SRC})
  message(${FILE})
endforeach()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC})

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  add_subdirectory(docs)
endif()

if(BUILD_TESTING
   AND CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME
   AND CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
  add_subdirectory(tests)
endif()
