# SConscript fuer DTrackSDK; A.R.T. 6.10.17-27.7.23

Import( "masterEnv" )

# create new build environment for customization
env = masterEnv.Clone()

# set include paths
dtrackSdkCppPath = [ Dir( "include" ).srcnode().abspath ]
env.AppendUnique( CPPPATH = dtrackSdkCppPath )

# link dependencies
if "msvc" in env[ "TOOLS" ]:
	env.Append( LIBS = [ "ws2_32" ] )

# custom compiler flags
if not "msvc" in env[ "TOOLS" ] :
	env.AppendUnique( CCFLAGS = "-fPIC" )  # to be able to link into shared objects

# get source and header files 
sourceFiles  = Glob( "src/lib/*.cpp" )
headerFiles = Glob( "include/*.hpp" ) + Glob( "include/*.h" )

# compile library
libName = "DTrackSDK"
lib = env.StaticLibrary( libName, sourceFiles )
masterEnv.RegisterLib( libName, lib, cpppath = dtrackSdkCppPath )

# create visual studio project file
env.AutoMsvsProject( libName, lib, sourceFiles, headerFiles )

