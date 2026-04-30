set( sources
	 example_communicating
	 example_dtrack1
	 example_fingertracking
	 example_listening
	 example_listening_multicast
	 example_tactile_flystick
	 example_universal
	 example_flystick_feedback
	 DTrackCLI )

set( headers SampleMath.h )

foreach( source ${sources} )
    if( DTRACKSDK_BUILD_EXAMPLES )

		if( DTRACKSDK_BUILD_LIBRARY )
        	add_executable( ${source} ${source}.cpp )
			target_link_libraries( ${source} DTrackSDK_LIB )
		else()
        	add_executable( ${source} ${source}.cpp ${DTrackSDK_src} )
		endif()

		if (WIN32)
			target_link_libraries(${source} ws2_32)
			target_link_libraries(${source} Winmm)
		endif()

		install( TARGETS ${source} DESTINATION bin COMPONENT examples )
		set_target_properties( ${source} PROPERTIES FOLDER examples )
    endif()
	
	install( FILES ${source}.cpp DESTINATION examples COMPONENT examples )
endforeach(source)

foreach( header ${headers} )
	install( FILES ${header} DESTINATION examples COMPONENT examples )
endforeach( header )

