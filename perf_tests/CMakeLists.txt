include(ProjectBenchmark)

list(APPEND PROJECT_PERF_TEST_SOURCES
    ${PROJECT_SOURCE_DIR}/perf_tests/PerfTestRunner.cpp
    ${PROJECT_SOURCE_DIR}/perf_tests/LogLogBetaPerfTests.cpp
    ${PROJECT_SOURCE_DIR}/perf_tests/LibCountPerfTests.cpp

    #libcount
    ${PROJECT_SOURCE_DIR}/perf_tests/extern/libcount/count/empirical_data.cc
    ${PROJECT_SOURCE_DIR}/perf_tests/extern/libcount/count/hll.cc
    ${PROJECT_SOURCE_DIR}/perf_tests/extern/libcount/count/hll_data.cc
    ${PROJECT_SOURCE_DIR}/perf_tests/extern/libcount/count/utility.cc
    )

project_add_perf_test(PerfTestRunner "${PROJECT_PERF_TEST_SOURCES}" "${PROJECT_NAME}_shared" )
