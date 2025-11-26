cmake_minimum_required(VERSION 3.11)

# Detect ESP-IDF build
if(DEFINED ENV{IDF_PATH})
    file(GLOB_RECURSE SRC src/*.cpp)
    set(COMPONENT_SRCS ${SRC}) # Add all your source files here
    set(COMPONENT_ADD_INCLUDEDIRS src)
    set(COMPONENT_PRIV_INCLUDEDIRS "")
    set(COMPONENT_REQUIRES "")
    set(COMPONENT_PRIV_REQUIRES "")
    register_component()
else()
    include(FetchContent)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

    project(DCCEXProtocol LANGUAGES CXX)

    file(GLOB_RECURSE SRC src/*.cpp)
    add_library(DCCEXProtocol STATIC ${SRC})
    add_library(DCCEX::Protocol ALIAS DCCEXProtocol)

    # Stuck at C++11
    target_compile_options(DCCEXProtocol PRIVATE -std=c++11)

    # Don't bother users with warnings by setting 'SYSTEM'
    if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
      target_include_directories(DCCEXProtocol PUBLIC src)
    else()
      target_include_directories(DCCEXProtocol SYSTEM PUBLIC src)
    endif()

    foreach(FILE ${SRC})
      message(${FILE})
    endforeach()

    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC})
endif()
