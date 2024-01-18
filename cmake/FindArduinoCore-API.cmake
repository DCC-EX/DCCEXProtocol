file(GLOB_RECURSE SRC ${arduinocore-api_SOURCE_DIR}/api/*.cpp)
add_library(ArduinoCore-API STATIC ${SRC})
target_include_directories(ArduinoCore-API
                           PUBLIC ${arduinocore-api_SOURCE_DIR}/api)
target_compile_definitions(ArduinoCore-API PUBLIC -DHOST)
