cmake_minimum_required(VERSION 3.8)

project(DCCEXProtocol)

set(PROJECT_SOURCE_DIR "src")

file(GLOB SOURCE_FILES "${PROJECT_SOURCE_DIR}/*.cpp" "${PROJECT_SOURCE_DIR}/*.h")

foreach(file ${SOURCE_FILES})
  message(${file})
endforeach()

source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_library(DCCEXProtocol ${SOURCE_FILES})

find_package(Doxygen)

if(DOXYGEN_FOUND)
  message("Found Doxygen")
  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.out)

  # request to configure the file
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
  message("Doxygen build started")

  # Note: do not put "ALL" - this builds docs together with application EVERY TIME!
  add_custom_target( docs
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif(DOXYGEN_FOUND)
