cmake_minimum_required(VERSION 3.11)

# Detect ESP-IDF build
if(ESP_PLATFORM OR DEFINED ENV{IDF_PATH})
  file(GLOB_RECURSE SRC src/*.cpp)
  idf_component_register(
    SRCS ${SRC}
    INCLUDE_DIRS src
    REQUIRES esp_timer
  )
else()
    include(FetchContent)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

    project(DCCEXProtocol LANGUAGES CXX)

    option(DCCEX_BUILD_TESTS "Build native GoogleTest test target" ON)

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

    if(DCCEX_BUILD_TESTS)
      enable_testing()

      FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
      )
      FetchContent_MakeAvailable(googletest)

      file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS test/*.cpp)
      add_executable(DCCEXProtocolTests ${TEST_SOURCES})

      # Compile the library with NATIVE_TESTING so it uses DCCStream.h instead of Arduino.h's Stream
      target_compile_definitions(DCCEXProtocol PRIVATE NATIVE_TESTING)

      target_compile_definitions(DCCEXProtocolTests PRIVATE NATIVE_TESTING)
      target_include_directories(DCCEXProtocolTests PRIVATE test/mocks test/setup)
      target_link_libraries(DCCEXProtocolTests PRIVATE DCCEXProtocol GTest::gtest GTest::gmock)

      include(GoogleTest)
      gtest_discover_tests(DCCEXProtocolTests)

      add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS DCCEXProtocolTests
        USES_TERMINAL
      )

      add_custom_target(run-tests DEPENDS check)
    endif()
endif()