include(ProjectGtest)

list(APPEND PROJECT_TEST_SOURCES
    ${PROJECT_SOURCE_DIR}/tests/TestRunner.cpp
    ${PROJECT_SOURCE_DIR}/tests/LogLogBetaTests.cpp
    )
project_add_test(TestRunner "${PROJECT_TEST_SOURCES}" "${PROJECT_NAME}_shared" )
