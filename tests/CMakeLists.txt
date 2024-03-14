include(${CMAKE_SOURCE_DIR}/cmake/sanitize.cmake)

file(GLOB_RECURSE SRC *.cpp)
add_executable(DCCEXProtocolTests ${SRC})

# Reuse ArduinoCore-API fakes and mocks
target_sources(
  DCCEXProtocolTests
  PRIVATE ${arduinocore-api_SOURCE_DIR}/test/src/MillisFake.cpp
          ${arduinocore-api_SOURCE_DIR}/test/src/PrintMock.cpp
          ${arduinocore-api_SOURCE_DIR}/test/src/StreamMock.cpp)
target_include_directories(DCCEXProtocolTests
                           PUBLIC ${arduinocore-api_SOURCE_DIR}/test/include)

sanitize(address,undefined)

target_compile_options(DCCEXProtocolTests PUBLIC -std=c++2a -Wall -Wextra
                                                 -Wconversion -Wsign-conversion)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG main)
FetchContent_MakeAvailable(googletest)

target_link_libraries(DCCEXProtocolTests PRIVATE DCCEXProtocol
                                                 GTest::gtest_main GTest::gmock)
