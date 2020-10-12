# use ccache for caching compilation if it is available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

# set cpu arch flags
set(CPU_FLAGS "-march=native -Ofast")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fno-omit-frame-pointer -DXXH_STATIC_LINKING_ONLY=1 ${CPU_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -O0 -ggdb -g3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -Ofast")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    # global compiler flags
    add_compile_options(-fcaret-diagnostics -fcolor-diagnostics)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")

    # global compiler flags
    add_compile_options(-fdiagnostics-color=always)
endif()
