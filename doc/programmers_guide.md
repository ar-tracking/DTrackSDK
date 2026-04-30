

# _DTrackSDK C++_ Build and Integration Guide

![](rsc/empty-30.png)

# 1 DTrackSDK
## Purpose of DTrackSDK
A set of functions to provide an interface to ART tracking systems. It supports all versions of DTRACK: DTrack(1), DTrack2, DTRACK3 and DTRACK4.

The functions receive and process DTRACK measurement data packets (UDP; ASCII), and send/exchange DTRACK command strings (UDP/TCP; ASCII).

## License
This library is distributed under the BSD 3-clause License. You can modify the sources
and/or include them into your own software (for details see `license.txt`).

## How to receive and process ART tracking data
All versions of DTRACK use ethernet (UDP/IP datagrams) to send measurement data
to other applications. They use an ASCII data format.

In its most simple operating mode DTrackSDK is just receiving and processing these data.
In this case DTrackSDK just needs to know the port number where the data are arriving; all necessary settings have to be done manually in the DTRACK frontend software.

DTRACK also provides a way to control the tracking system through a
command interface via ethernet. Both DTrack(1) and DTrack2/DTRACK3/DTRACK4 use ASCII command strings. DTrack2/DTRACK3/DTRACK4 commands are sent via a TCP/IP connection.

DTrackSDK offers several C++ constructors to cover the various operating modes; prefer
the “universal constructor” to obtain the best flexibility.

The formats and all other necessary definitions are described in “DTrack2 User Manual, Technical Appendix” or “DTRACK3/DTRACK4 Programmer’s Guide”.

## Sample source codes for an own interface
The sample source code files in the `examples` directory show how to use the DTrackSDK:

file | description
-|-
example_universal.cpp| sample using the universal constructor for all modes
example_listening.cpp| sample for pure listening
example_listening_multicast.cpp| sample for pure multicast listening
example_communicating.cpp| sample to control a DTrack2/DTRACK3 Controller or DTRACK4 PC
example_fingertracking.cpp| ART FINGERTRACKING sample
example_tactile_flystick.cpp| sample using a Flystick to control a tactile FINGERTRACKING device
example_flystick_feedback.cpp| sample to control a Flystick with feedback
example_dtrack1.cpp| sample to control a DTrack(1) PC

Each example uses a different constructor and explains how to use it.
All examples are written in C++, and work for both Unix and Windows. The files have
been successfully tested under Linux, Windows 7, Windows 10 and Windows 11.

For Windows: please link with library “ws2_32.lib”.

For Windows: example_tactile_flystick.cpp needs to be linked with “winmm.lib”.

For older Linux: example_tactile_flystick.cpp needs to be linked with “-lrt” (only for glibc
versions before 2.17).


## Compatibility with legacy SDK versions
DTrackSDK comes with additional sources that should make it easier to upgrade from older SDK versions. Actually they are just wrappers around class “DTrackSDK”, and allow to use the known interface. They are available for the classes:

class||
-|-
DTracklib| class esp. for DTrack(1) systems (C++; used from May 2005 until Dec 2006)
DTrack| class esp. for DTrack(1) systems (C++; used from Jan 2007 until Dec 2010)
DTrack2| class esp. for DTrack2 systems (C++; used from May 2008 until Dec 2010)

Note that the wrappers just work for the C++ SDK versions; there’s no wrapper for the
“DTracklib” SDK written in C.


## Contents of this package
content||
-|-
LICENSE| copy of BSD 3-clause License
include/| C++ headers
src/| C++ implementations and Python wrapper
examples/| C++ sample sources (see above)
Compatibility/| C++ wrappers for compatibility with legacy SDK classes
doc/html/| HTML documentation generated with doxygen (main file is “index.html”)




## Development with DTrackSDK

### Integrate and Build with CMake
You can keep the DTrackSDK repository as a CMake sub-project, e.g.

```
YourProject/
- CMakeLists.txt
- ...
- extern/
  - DTrackSDK/
	- CMakeLists.txt
	- ...
```

and then add something like this to an existing CMake setup:
	
```
add_subdirectory( extern/DTrackSDK )
target_link_libraries( ${PROJECT_NAME} DTrackSDK_LIB )
```

This would build and link the SDK as a static library without examples or Python wrapper using the default values

```
DTRACKSDK_BUILD_LIBRARY  = ON
DTRACKSDK_STATIC_LIBRARY = ON
DTRACKSDK_BUILD_EXAMPLES = OFF
DTRACKSDK_PYBIND_ROOT    = ""
```


### Direct Source Integration
a) __Create a new project using the DTrackSDK:__
- include the header file:
  - include/DTrackSDK.hpp
- add following include paths:
  - include/
- add following source files to your project:
  - src/lib/DTrackSDK.cpp
  - src/lib/DTrackData.cpp
  - src/lib/DTrackNet.cpp
  - src/lib/DTrackParse.cpp
  - src/lib/DTrackParser.cpp
- you may want to start with one of the example files provided in this package
- if you’re unsure which C++ constructor to use best, take the “universal constructor”
(like shown in “example_universal.cpp”); it provides the best flexibility

b) __Upgrade an existing project developed with legacy DTrack SDK versions:__
- upgrade from class “DTracklib”:
  - include the header file:
    - Compatibility/DTracklib/DTracklib.hpp
  - add following include paths:
    - include/
    - Compatibility/DTracklib/
  - add following source files to your project:
    - src/lib/DTrackSDK.cpp
    - src/lib/DTrackData.cpp
    - src/lib/DTrackNet.cpp
    - src/lib/DTrackParse.cpp
    - src/lib/DTrackParser.cpp
    - Compatibility/DTracklib/DTracklib.cpp
- upgrade from class “DTrack”:
  - include the header file:
    - Compatibility/DTrack/DTrack.hpp
  - add following include paths:
    - include/
    - Compatibility/DTrack/
  - add following source files to your project:
    - src/lib/DTrackSDK.cpp
    - src/lib/DTrackData.cpp
    - src/lib/DTrackNet.cpp
    - src/lib/DTrackParse.cpp
    - src/lib/DTrackParser.cpp
    - Compatibility/DTrack/DTrack.cpp
- upgrade from class “DTrack2”:
  - include the header file:
    - Compatibility/DTrack/DTrack2.hpp
  - add following include paths:
    - include/
    - Compatibility/DTrack2/
  - add following source files to your project:
    -  src/lib/DTrackSDK.cpp
    -  src/lib/DTrackData.cpp
    -  src/lib/DTrackNet.cpp
    -  src/lib/DTrackParse.cpp
    -  src/lib/DTrackParser.cpp
    -  Compatibility/DTrack2/DTrack2.cpp





# 2 Detailed Instructions

## 2.1 Build examples with CMake (Windows/Linux)

Clone the DTrackSDK repository from [`ar-tracking/DTrackSDK`](https://github.com/ar-tracking/DTrackSDK), or extract the contents of DTrackSDK_v2.10.0.zip if available.
Enter the corresponding DTrackSDK project root directory in your Windows console / Linux terminal and build the examples with the following commands:

```
$ mkdir build; cd build
$ cmake .. -DDTRACKSDK_BUILD_LIBRARY=OFF -DDTRACKSDK_BUILD_EXAMPLES=ON
$ cmake --build .
```

## 2.1 Build examples on Windows (no CMake)

### Used in this example
- Windows 10 64-bit
- Visual Studio 2022
- DTrackSDK_v2.10.0.zip

### How to build an example
1. Create an empty project
2. Copy following files from “DTrackSDK_v2.10.0.zip” to your project folder
    - a) complete “include” folder
    - b) complete “src/lib” folder
    - c) “examples/example_universal.cpp”
3. Include the files to the project in Visual Studio
    - Project → Add Existing Item. . .
    - (Take care that the project is selected)
4. Project → Properties → Configuration Properties → Linker → Input
    - include at “Additional Dependencies”: ws2_32.lib and press enter
	- _Note_ that example_tactile_flystick.cpp needs also to be linked with “winmm.lib”.
5. Project → Properties → Configuration Properties → Linker → System
    - set “SubSystem” to: Console (/SUBSYSTEM:CONSOLE)
6. Project → Properties → Configuration Properties → VC++ Directories → Include Directories → <Edit...> → New Line (Ctrl-Insert) → Browse... → Point to the "DTrackSDK/include" path in DTrackSDK installation
7. Build the example

**Steps 4, 5 & 6 have to be done in debug and release mode**

The other examples can be built similarly.


## 2.2 Build examples on Linux (no CMake)
### Used in this example
-  Ubuntu 24.04
-  GNU Compiler Collection g++ 13.3
-  DTrackSDK_v2.10.0.zip


### How to build an example
1. Unzip DTrackSDK_v2.10.0.zip
2. Copy following files from “DTrackSDK_v2.10.0/” to your project folder
    - a) complete “include” folder
    - b) complete “src/lib” folder
    - c) “examples/example_universal.cpp”
3. Create a new file named “Makefile” in the project folder
4. Copy and paste the following code into the Makefile. Ensure to include new lines as shown, substitute any spaces before `g++ ...` with one tab, then save the file
   ```Makefile
   all: examples/example_universal
   %: %.cpp
      g++ -o $* -I include/ $< src/lib/*.cpp
   ```
5. Run `$ make` in the terminal

The other examples can be built similarly.


### How to use the compiled examples
1. Start the measurement of the ART Tracking System with the following settings
	- One or more bodies are calibrated and tracked
	- Output settings: 
		- Send to → this computer
		- Port 5000 (make sure that your system’s firewall doesn’t block this port)
2. Start the program in the Windows console or Linux terminal, e.g. `example_universal 5000`
3. Tracking data are displayed

The other examples can be used similarly.



\newpage


## Trademarks
The following overview shows the registered trademarks of Advanced Realtime Tracking GmbH & Co. KG:

| trademarks    | illustrated as| **in Germany**| **in the EU**| **in the U.S.**|
|---------------|---------------|:-------------:|:------------:|:--------------:|
|**A.R.T.®**    | ART           | ×             |              |                |
|**ART®**       | ART           | ×             | ×            | ×              |
|**ARTtrack®**  | ARTTRACK      | ×             | ×            | ×              |
|**DTrack®**    | DTRACK        | ×             |              |                |
|**smARTtrack®**| SMARTTRACK    | ×             |              | ×              |
|![logo](rsc/logo-white.png) |  | ×             | ×            | ×              |
|![logo](rsc/logo-black.png) |  | ×             | ×            | ×              |

Microsoft® and Windows® are trademarks registered in the United States and other countries by the Microsoft Corporation.

The company names and product names written in this manual are trademarks or registered trademarks of the respective companies

![](rsc/empty-30.png)
![](rsc/empty-30.png)

\newpage


|![](rsc/logo-white.png)|
|:-:|
|Advanced Realtime Tracking GmbH & Co. KG
|Am Öferl 6
|82362 Weilheim i. OB
|Germany
|T +49 (0)881-92530-00
|v +49 (0)881-92530-01
|https://ar-tracking.com

