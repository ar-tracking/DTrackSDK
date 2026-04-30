

if(DTRACKSDK_BUILD_DOCPDF)

	find_program( PANDOC_EXECUTABLE NAMES pandoc )
	find_program( PDFLATEX_EXECUTABLE NAMES pdflatex )

	if( NOT PANDOC_EXECUTABLE )
		message( FATAL_ERROR "Could not find pandoc" )
	endif()

	if( NOT PDFLATEX_EXECUTABLE )
		message( FATAL_ERROR "Could not find pdflatex" )
	endif()


	set( DOC_NAME "programmers_guide" )
	set( ABS_PATH_PDF "${CMAKE_CURRENT_BINARY_DIR}/${DOC_NAME}.pdf" )

	add_custom_command(
		OUTPUT ${ABS_PATH_PDF}
		COMMAND ${PANDOC_EXECUTABLE}
			"${CMAKE_CURRENT_LIST_DIR}/${DOC_NAME}.md"
			"${CMAKE_CURRENT_LIST_DIR}/rsc/${DOC_NAME}.yaml"
			"--pdf-engine=pdflatex"
			"-o" "${ABS_PATH_PDF}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
		DEPENDS
			"${CMAKE_CURRENT_LIST_DIR}/${DOC_NAME}.md"
			"${CMAKE_CURRENT_LIST_DIR}/rsc/${DOC_NAME}.yaml"
			"${CMAKE_CURRENT_LIST_DIR}/rsc/logo-white.png"
			"${CMAKE_CURRENT_LIST_DIR}/rsc/logo-black.png"
		COMMENT "Generating ${ABS_PATH_PDF} ..."
		VERBATIM
	)

	add_custom_target( doc_pdf ALL
		DEPENDS ${ABS_PATH_PDF}
	)

endif(DTRACKSDK_BUILD_DOCPDF)


# TODO, use doxygen_add_docs
if( DTRACKSDK_BUILD_DOXYGEN)

	find_package(Doxygen REQUIRED)
    set(DOXYGEN_IN  "${CMAKE_CURRENT_LIST_DIR}/doxygen.conf.in")
    set(DOXYGEN_CFG "${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf")

    configure_file("${DOXYGEN_IN}" "${DOXYGEN_CFG}" @ONLY)

	set(DOXYGEN_STAMP "${CMAKE_CURRENT_BINARY_DIR}/doxygen_html.stamp")

    add_custom_command(
        OUTPUT  "${DOXYGEN_STAMP}"
        COMMAND ${DOXYGEN_EXECUTABLE} "${DOXYGEN_CFG}"
        COMMAND ${CMAKE_COMMAND} -E touch "${DOXYGEN_STAMP}"
        DEPENDS "${DOXYGEN_CFG}" "${DOXYGEN_IN}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Generating API documentation..."
        VERBATIM
    )

    add_custom_target(doc_doxygen ALL
        DEPENDS "${DOXYGEN_STAMP}"
    )
	
endif(DTRACKSDK_BUILD_DOXYGEN)

