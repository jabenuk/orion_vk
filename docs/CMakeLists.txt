# ====================================================== #
#                 Orion CMake lists: DOCS                #
# ====================================================== #

#
# verify Doxygen installation

find_package(Doxygen)

if (DOXYGEN_FOUND)
    set(DOXYGEN_IN "${ORION_DOCS_DIR}/Doxyfile.cmakeinput")
    set(DOXYGEN_OUT "${PROJECT_BINARY_DIR}/docs/Doxyfile")

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(${PROJECT_NAME}docs
        ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
else()
    message(WARNING "Doxygen package not found, skipping generation of API documentation!")
endif()
