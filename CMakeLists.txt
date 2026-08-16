cmake_minimum_required(VERSION 3.20)

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
    set(CMAKE_CXX_EXTENSIONS OFF)

    option(DCCEX_BUILD_TESTS "Build native GoogleTest test target" ON)
    option(DCCEX_BUILD_EXAMPLES "Build native protocol examples" OFF)

    file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.cpp)

    function(configure_dccex_target TARGET_NAME)
      # Keep the public library compatible with the C++11 baseline used by the
      # Arduino ecosystem. CMake selects the correct compiler-specific flags.
      target_compile_features(${TARGET_NAME} PUBLIC cxx_std_11)

      # Don't bother users with warnings by setting 'SYSTEM'.
      if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        target_include_directories(${TARGET_NAME} PUBLIC src)
      else()
        target_include_directories(${TARGET_NAME} SYSTEM PUBLIC src)
      endif()
    endfunction()

    add_library(DCCEXProtocol STATIC ${SRC})
    add_library(DCCEX::Protocol ALIAS DCCEXProtocol)
    configure_dccex_target(DCCEXProtocol)

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
      add_library(DCCEXProtocolTest STATIC ${SRC})
      configure_dccex_target(DCCEXProtocolTest)

      # Keep the production library on the host clock while compiling a
      # separate test-library instance against the deterministic Arduino mock.
      target_compile_definitions(DCCEXProtocolTest PRIVATE NATIVE_TESTING)
      target_include_directories(DCCEXProtocolTest PRIVATE test/mocks test/setup)

      add_executable(DCCEXProtocolTests ${TEST_SOURCES})
      target_compile_definitions(DCCEXProtocolTests PRIVATE NATIVE_TESTING)
      target_include_directories(DCCEXProtocolTests PRIVATE test/mocks test/setup)
      target_link_libraries(DCCEXProtocolTests PRIVATE DCCEXProtocolTest GTest::gtest GTest::gmock)

      include(GoogleTest)
      gtest_discover_tests(DCCEXProtocolTests)

      add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS DCCEXProtocolTests
        USES_TERMINAL
      )

      add_custom_target(run-tests DEPENDS check)
    endif()

    if(DCCEX_BUILD_EXAMPLES)
      add_executable(DCCEXProtocolNativeExample examples/native_basic/main.cpp)
      target_compile_features(DCCEXProtocolNativeExample PRIVATE cxx_std_11)
      target_link_libraries(DCCEXProtocolNativeExample PRIVATE DCCEX::Protocol)
    endif()
endif()
