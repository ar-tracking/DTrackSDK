set(TARGET_NAME DTrackSDK_LIB)
set(LIBRARY_NAME "DTrackSDK")

install(FILES ${DTrackSDK_src} DESTINATION src COMPONENT sources)

if(DTRACKSDK_BUILD_LIBRARY)

	set(inc ${PROJECT_SOURCE_DIR}/include)
	file(GLOB headers "${inc}/*.h*")
			
	add_library(${TARGET_NAME}
		${DTRACKSDK_LIBRARY_TYPE}
		${DTrackSDK_src}
		${headers}
	)
	target_include_directories(${TARGET_NAME} PUBLIC ${inc})

	if (WIN32)
		set_target_properties( ${TARGET_NAME} 
			PROPERTIES 
			OUTPUT_NAME ${LIBRARY_NAME}
			VERSION  ${DTRACKSDK_VERSION}
			FOLDER lib )

		target_link_libraries(${TARGET_NAME} PRIVATE ws2_32)
		target_link_libraries(${TARGET_NAME} PRIVATE Winmm)

		if (MSVC AND NOT DTRACKSDK_STATIC_LIBRARY)
			set_target_properties( ${TARGET_NAME} PROPERTIES
				WINDOWS_EXPORT_ALL_SYMBOLS ON )  # TODO
		endif()

		
	else (WIN32)
		set_target_properties( ${TARGET_NAME} 
			PROPERTIES 
			OUTPUT_NAME ${LIBRARY_NAME}
			VERSION  ${DTRACKSDK_VERSION}
			FOLDER lib
			LINKER_LANGUAGE CXX
		)
	endif (WIN32)


	install(TARGETS ${TARGET_NAME}
		RUNTIME DESTINATION bin COMPONENT libraries
		ARCHIVE DESTINATION lib COMPONENT libraries
		LIBRARY DESTINATION lib COMPONENT libraries
	)

endif(DTRACKSDK_BUILD_LIBRARY)
