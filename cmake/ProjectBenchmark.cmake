set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/benchmark" "extern/benchmark")

set_target_properties(benchmark PROPERTIES FOLDER extern)

function(project_add_perf_test PERF_TESTNAME PERF_TEST_SOURCES PROJECT_LIBRARY)
    add_executable(${PERF_TESTNAME} ${PERF_TEST_SOURCES})
    target_link_libraries(${PERF_TESTNAME} benchmark ${PROJECT_LIBRARY})

    set_target_properties(${PERF_TESTNAME} PROPERTIES FOLDER perf_tests)

    target_include_directories(${PERF_TESTNAME} PUBLIC ${PROJECT_SOURCE_DIR}/perf_tests)

    add_custom_target(${PERF_TESTNAME}_run ALL
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${PERF_TESTNAME} --benchmark_color=true
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Running tests in ${CMAKE_CURRENT_BINARY_DIR}"
        SOURCES ${PERF_TEST_SOURCES}
    )

    add_dependencies(${PERF_TESTNAME}_run ${PERF_TESTNAME})
    if(NOT PROJECT_LIBRARY STREQUAL "")
        add_dependencies(${PERF_TESTNAME}_run ${PROJECT_LIBRARY})
    endif()
endfunction()
