macro(sanitize SANITIZERS)
  # Set in current scope
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=${SANITIZERS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=${SANITIZERS}")
  set(LDFLAGS "${LDFLAGS} -fsanitize=${SANITIZERS}")

  # Set in PARENT_SCOPE
  get_directory_property(HAS_PARENT_SCOPE PARENT_DIRECTORY)
  if(HAS_PARENT_SCOPE)
    set(CMAKE_C_FLAGS
        ${CMAKE_C_FLAGS}
        PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS
        ${CMAKE_CXX_FLAGS}
        PARENT_SCOPE)
    set(LDFLAGS
        ${LDFLAGS}
        PARENT_SCOPE)
  endif()
endmacro()
