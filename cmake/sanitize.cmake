macro(sanitize SANITIZERS)
  get_directory_property(HAS_PARENT_SCOPE PARENT_DIRECTORY)
  if(HAS_PARENT_SCOPE)
    set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} -fsanitize=${SANITIZERS}"
        PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -fsanitize=${SANITIZERS}"
        PARENT_SCOPE)
    set(LDFLAGS
        "${LDFLAGS} -fsanitize=${SANITIZERS}"
        PARENT_SCOPE)
  endif()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=${SANITIZERS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=${SANITIZERS}")
  set(LDFLAGS "${LDFLAGS} -fsanitize=${SANITIZERS}")
endmacro()
