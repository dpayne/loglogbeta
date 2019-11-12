cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0077 NEW)

set(PROJECT_NAME "llb")
set(PROJECT_VERSION "0.1.0")
set(PACKAGE_VERSION ${PROJECT_VERSION})

project(${PROJECT_NAME} VERSION ${PACKAGE_VERSION} LANGUAGES C CXX)
message(STATUS "Configuring ${CMAKE_PROJECT_NAME} ${PROJECT_VERSION}")

include(GNUInstallDirs)

# set module path for cmake, this is where all the cmake options go
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Some default options
include(DefineOptions)

# Compilation flags and includes
include(DefineFlags)

# Clang Sanitizers
include(ClangSanitizers)

# Clang Tidy
include(ClangTidyAnalysis)

# Clang Format
include(ClangFormat)

# project cmake
include(src/CMakeLists.txt)

if(BUILD_TESTS)
    # Tests
    include(tests/CMakeLists.txt)
endif()

if(BUILD_PERF_TESTS)
    # Performance Tests
    include(perf_tests/CMakeLists.txt)
endif()
