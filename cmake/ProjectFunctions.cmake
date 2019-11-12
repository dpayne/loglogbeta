function(add_project_library LIBRARY_TARGET_NAME LIBRARY_NAME LIBRARY_HEADERS LIBRARY_SOURCES LIBRARY_BUILD_TYPE)
    message("Building library ${LIBRARY_TARGET_NAME} with sources ${LIBRARY_SOURCES}")

    add_library(${LIBRARY_TARGET_NAME} ${LIBRARY_BUILD_TYPE} ${LIBRARY_SOURCES})

    set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES
        CXX_STANDARD 17
        LINKER_LANGUAGE CXX
        OUTPUT_NAME ${LIBRARY_NAME}
        )

    #project libraries
    #target_link_libraries(${LIBRARY_TARGET_NAME} lib1 lib2)

    # 'make install' to the correct locations (provided by GNUInstallDirs)
    install(TARGETS ${LIBRARY_TARGET_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endfunction()
