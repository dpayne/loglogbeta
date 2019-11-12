option(BUILD_TESTS "Comple and run tests" OFF)
option(BUILD_PERF_TESTS "Compile and run performance tests" OFF)
option(RUN_FORMATTER "Format codebase" OFF)
option(GENERATE_DOCS "Build documentation" OFF)
option(RUN_CLANG_TIDY "Run clang tidy on codebase" OFF)

# create compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(default_build_type "Release")
