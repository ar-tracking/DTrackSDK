if(EXISTS "${DTRACKSDK_PYBIND_ROOT}")

	# Dependencies
	set(PYBIND11_FINDPYTHON ON)
	find_package(Python COMPONENTS Interpreter Development REQUIRED)
	add_subdirectory(${DTRACKSDK_PYBIND_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/pybind)
	
	
	# Build
	pybind11_add_module( DTrackSDK
		${DTrackSDK_src}
		DTrackSDK-pybind.cpp
	)
	
	if(WIN32)
	    target_link_libraries( DTrackSDK PRIVATE 
	    	ws2_32 
	    	pybind11::module
	    )
	else(WIN32)
		target_link_libraries( DTrackSDK PRIVATE 
	    	pybind11::module
	    )
	endif(WIN32)
	
	
	# Install files
	install(TARGETS DTrackSDK
		RUNTIME DESTINATION python COMPONENT python
		LIBRARY DESTINATION python COMPONENT python
		ARCHIVE DESTINATION python COMPONENT python
	)

else()
	message(STATUS "Path DTRACKSDK_PYBIND_ROOT does not exist (${DTRACKSDK_PYBIND_ROOT})")
endif()



