include(ProjectFunctions)
include(ProjectExternalLibs)

# external libraries folder
link_directories(${EXTERNAL_LIBS_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/extern/xxHash/ ${EXTERNAL_INLCUDE_DIR})

list(APPEND PROJECT_HEADERS
    include/LogLogBeta.h
)

list(APPEND PROJECT_SOURCES
    src/LogLogBeta.cpp
)

add_project_library("${PROJECT_NAME}_static" "${PROJECT_NAME}" "${PROJECT_HEADERS}" "${PROJECT_SOURCES}" STATIC)
add_project_library("${PROJECT_NAME}_shared" "${PROJECT_NAME}" "${PROJECT_HEADERS}" "${PROJECT_SOURCES}" SHARED)

target_link_libraries("${PROJECT_NAME}_static" PRIVATE xxHash::xxhash)
target_link_libraries("${PROJECT_NAME}_shared" PRIVATE xxHash::xxhash)
