file(GLOB_RECURSE SRC ${arduinocore-api_SOURCE_DIR}/api/*.cpp)
add_library(ArduinoCore-API STATIC ${SRC})

# Prevent redeclaration of int atexit(void (*func)())
target_compile_definitions(ArduinoCore-API PUBLIC -DHOST)

target_include_directories(
  ArduinoCore-API PUBLIC ${arduinocore-api_BINARY_DIR}
                         ${arduinocore-api_SOURCE_DIR}/api)

# Create Arduino.h header
file(
  WRITE ${arduinocore-api_BINARY_DIR}/Arduino.h #
  "#ifndef Arduino_h\n" #
  "#define Arduino_h\n" #
  "#include \"ArduinoAPI.h\"\n" #
  "#endif" #
)
