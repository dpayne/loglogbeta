include(ProjectBenchmark)

list(APPEND PROJECT_PERF_TEST_SOURCES
    ${PROJECT_SOURCE_DIR}/perf_tests/PerfTestRunner.cpp
    ${PROJECT_SOURCE_DIR}/perf_tests/LogLogBetaPerfTests.cpp
    )

project_add_perf_test(PerfTestRunner "${PROJECT_PERF_TEST_SOURCES}" "${PROJECT_NAME}_shared" )
