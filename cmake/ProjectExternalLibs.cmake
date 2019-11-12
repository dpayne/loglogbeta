set(XXHASH_BUILD_ENABLE_INLINE_API OFF) #optional
set(XXHASH_BUILD_XXHSUM OFF) #optional
add_subdirectory(${PROJECT_SOURCE_DIR}/extern/xxHash/cmake_unofficial extern/xxHash EXCLUDE_FROM_ALL)
