add_subdirectory("${PROJECT_SOURCE_DIR}/extern/googletest" "extern/googletest")

mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
    )

set_target_properties(gtest PROPERTIES FOLDER extern)
set_target_properties(gtest_main PROPERTIES FOLDER extern)
set_target_properties(gmock PROPERTIES FOLDER extern)
set_target_properties(gmock_main PROPERTIES FOLDER extern)

function(project_add_test TESTNAME TEST_SOURCES PROJECT_LIBRARY)
    add_executable(${TESTNAME} ${TEST_SOURCES})
    target_link_libraries(${TESTNAME} gtest gmock gtest_main ${PROJECT_LIBRARY})
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)

    add_custom_target(${TESTNAME}_run ALL
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME} --gtest_color=yes --gtest_output=xml:${PROJECT_SOURCE_DIR}/gtestresults.xml
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Running tests in ${CMAKE_CURRENT_BINARY_DIR}"
        SOURCES ${TEST_SOURCES}
    )

    add_dependencies(${TESTNAME}_run ${PROJECT_LIBRARY})
endfunction()
