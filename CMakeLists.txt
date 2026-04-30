cmake_minimum_required(VERSION 3.18)
project(DTrackSDK LANGUAGES CXX C)

# This version setting does not affect DTrackSDKVersion.hpp
set( DTRACKSDK_VERSION 2.10.0)

# Options
set(DTRACKSDK_LIBRARY_TYPE SHARED)

option(DTRACKSDK_BUILD_LIBRARY  "Build SDK library"        ON)
option(DTRACKSDK_BUILD_EXAMPLES "Build examples"           OFF)
option(DTRACKSDK_PYBIND_ROOT    "Path to PyBind11 source"  "")
option(DTRACKSDK_BUILD_DOCPDF   "Generate PDF docs"        OFF) 
option(DTRACKSDK_BUILD_DOXYGEN  "Generate Doxygen"         OFF) 


if(DTRACKSDK_BUILD_LIBRARY)
	option(DTRACKSDK_STATIC_LIBRARY "Build static library" ON)
	if(DTRACKSDK_STATIC_LIBRARY)
	
		set(DTRACKSDK_LIBRARY_TYPE STATIC)
		
		if (NOT WIN32)
			add_definitions(-fPIC)
		endif (NOT WIN32)
		
	endif(DTRACKSDK_STATIC_LIBRARY)
endif(DTRACKSDK_BUILD_LIBRARY)


# Build

set ( DTrackSDK_src ${PROJECT_SOURCE_DIR}/src/lib/DTrackNet.cpp
	                ${PROJECT_SOURCE_DIR}/src/lib/DTrackData.cpp
	                ${PROJECT_SOURCE_DIR}/src/lib/DTrackParse.cpp
	                ${PROJECT_SOURCE_DIR}/src/lib/DTrackParser.cpp
	                ${PROJECT_SOURCE_DIR}/src/lib/DTrackSDK.cpp )

include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(examples)

if(DTRACKSDK_BUILD_DOCPDF OR DTRACKSDK_BUILD_DOXYGEN)
	add_subdirectory(doc)
endif()





