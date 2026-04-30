
# DTrackSDK

A set of functions to provide an interface (C++ and Python) to [_A.R.T. tracking systems_][1]. 
It supports all versions of DTRACK: DTrack(1), DTrack2, DTRACK3 and DTRACK4. 
The functions receive and process DTRACK measurement data packets (UDP; ASCII), and send/exchange DTRACK command strings (UDP/TCP; ASCII).


----------------------------------------------
## Building with C++ Projects

If you want the DTrackSDK C++ source to be directly integrated into and compiled with your C++ source without CMake, just copy the files `src/lib/*.cpp` and `include/*.hpp` (but _not_ `src/python/*`). On Windows you have to manually satisfy the linker dependency on `winmm.lib` and `ws2_32.lib`.

Alternatively you can keep the DTrackSDK repository as a CMake sub-project, e.g.

```
YourProject/
├ CMakeLists.txt
├ ...
└ extern/
  └ DTrackSDK/
	├ CMakeLists.txt
	└ ...
```

and then add something like this to an existing CMake setup:
	
```
add_subdirectory( extern/DTrackSDK )
target_link_libraries( ${PROJECT_NAME} DTrackSDK_LIB )
```

This would build and link the SDK as a static library without examples or Python wrapper using the default values for the CMake build switches

```
DTRACKSDK_BUILD_LIBRARY  = ON
DTRACKSDK_STATIC_LIBRARY = ON
DTRACKSDK_BUILD_EXAMPLES = OFF
DTRACKSDK_PYBIND_ROOT    = ""
```

See [__`doc/programmers_guide.md`__][3] for details about building the SDK and the examples.


----------------------------------------------
## Using the C++ SDK

The source in the [`examples`][4] directory offers a straightforward way to understand the SDK usage. It covers the following use cases:

file | description
-|-
`example_universal.cpp` 			| sample using the universal constructor for all modes
`example_listening.cpp` 			| sample for pure listening
`example_listening_multicast.cpp` 	| sample for pure multicast listening
`example_communicating.cpp` 		| sample to control a DTrack2/DTRACK3 Controller or DTRACK4 PC
`example_fingertracking.cpp` 		| ART FINGERTRACKING sample
`example_tactile_flystick.cpp` 		| sample using a Flystick to control a tactile FINGERTRACKING device
`example_flystick_feedback.cpp` 	| sample to control a Flystick with feedback
`example_dtrack1.cpp` 				| sample to control a DTrack(1) PC

`example_listening.cpp` is probably the easiest example to get started with. 
Note that in a real application you might prefer to move SDK-related code to a separate thread.

See [__`doc/programmers_guide.md`__][3] to learn more about SDK examples.


----------------------------------------------
## Building for Python

Download [PyBind11][2] if you want to build the Python wrapper for DTrackSDK.
Extract the code to a directory of your choice, e.g. `your/path/to/pybind11/`.

Note that PyBind11 comes with its own license.
Also note that PyBind11 and the corresponding DTrackSDK Python wrapper require at least C++11.

To build for the Python version of the Python executable found in the current path go to the DTrackSDK root directory and use (with absolute paths)

```
$ mkdir build; cd build
$ cmake .. -DDTRACKSDK_PYBIND_ROOT=your/path/to/pybind11 -DDTRACKSDK_BUILD_LIBRARY=OFF
$ cmake --build .
``` 



----------------------------------------------
## Using the Python SDK

Note that the Python wrapper requires `numpy` to be installed; you can achive this e.g. via `python -m pip install numpy` using the Python executable you want to use the wrapper with.

Once built you find the wrapper as a file named like `DTrackSDK.cpython-312-x86_64-linux-gnu.so` (Linux) or `DTrackSDK.cp312-win_amd64.pyd` (Windows) in the `build/src/python` subfolder (the `build` subfolder used above). Copy this file to your `sys.path`.

The wrapper currently supports a subset of the C++ SDK functionality, namely _Body_ `6d`, _Flystick_ `6df2` and _Fingertracking_ `gl` data, without feedback.
It provides an API simliar to the C++ SDK version.

To test the wrapper e.g. start your [_A.R.T. tracking system_][1] with output port `5000` and a tracked Body with DTrack ID `1` (which corresponds to ID `0` from the SDK point of view), then try:

```
$ python
>>> import DTrackSDK
>>> DTrackSDK.__version__         # example output
'2.10.0'
>>> import numpy as np
>>> dt = DTrackSDK.dtrack( 5000)
>>> dt.receive()
True
>>> bod = dt.getBody(0)
>>> bod.isTracked()
True
>>> bod.loc                       # example output
array([[259.317],
       [409.852],
       [826.892]])
>>> bod.rot                       # example output
array([[-0.755817,  0.184922, -0.628128],
       [-0.635247,  0.025491,  0.771888],
       [ 0.15875 ,  0.982423,  0.098204]])
```

----------------------------------------------
## Generate documentation

If you prefer the [__`doc/programmers_guide.md`__][3] as PDF instead of Markdown, build it with 

```
$ mkdir build; cd build
$ cmake .. -DDTRACKSDK_BUILD_LIBRARY=OFF -DDTRACKSDK_BUILD_DOCPDF=ON
$ cmake --build .
```

This requires [_Pandoc_][5] and a suitable _LaTeX_ variant like [_MikTex_][6] to be installed, as well as the commands `pandoc` and `pdflatex` beeing accessible through your path environment variable.


Similarly you can build an API documentation of the SDK, provided [_Doxygen_][7] is installed and available in your path

```
$ cmake .. -DDTRACKSDK_BUILD_LIBRARY=OFF -DDTRACKSDK_BUILD_DOXYGEN=ON
```


----------------------------------------------
## License

See [`./license.txt`](license.txt)


----------------------------------------------
## Company details

Advanced Realtime Tracking GmbH & Co. KG
<br>Am Oeferl 6
<br>D-82362 Weilheim
<br>Germany

https://ar-tracking.com/en




----------------------------------------------
[1]: https://ar-tracking.com/en
[2]: https://github.com/pybind/pybind11/releases
[3]: doc/programmers_guide.md
[4]: examples
[5]: https://pandoc.org/installing.html
[6]: https://miktex.org/download
[7]: https://www.doxygen.nl/download.html



