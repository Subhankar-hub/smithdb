# Included by CMake through CMAKE_USER_MAKE_RULES_OVERRIDE, after the compiler module sets its
# default *_INIT flags and before they are written to the cache. Explicit -DCMAKE_<LANG>_FLAGS_<CONFIG>
# values given by the user still take precedence.
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_C_FLAGS_DEBUG_INIT "-O0 -g")
    set(CMAKE_C_FLAGS_RELEASE_INIT "-O2 -DNDEBUG")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O2 -DNDEBUG")
endif()
