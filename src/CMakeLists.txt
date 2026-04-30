
if(DTRACKSDK_BUILD_LIBRARY)
	add_subdirectory(lib)
endif()


# Build python binding
if(NOT "${DTRACKSDK_PYBIND_ROOT}" STREQUAL "")
	add_subdirectory(python)
endif()
