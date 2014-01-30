if(MSVC)
    add_definitions(-DFCAppCam -DHAVE_ACOSH -DHAVE_ASINH -DHAVE_ATANH)
else(MSVC)
    add_definitions(-DHAVE_LIMITS_H -DHAVE_CONFIG_H -fPIC)
endif(MSVC)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Boost_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_PATH}
)

if(MSVC)
	# We must exclude 'boost_python' for the windows build due to the naming
	# of the library.  The boost python library gets linked anyway but
	# it has the correct filename used.
	set(AreaClipper_LIBS
	    ${PYTHON_LIBRARIES}
	    ${Boost_LIBRARIES})
else(MSVC)
	set(AreaClipper_LIBS
	    boost_python
	    ${PYTHON_LIBRARIES}
	    ${Boost_LIBRARIES})
endif(MSVC)

#
# # Turn compiler warnings up to 11, at least with gcc.
# if (CMAKE_BUILD_TOOL MATCHES "make")
#     MESSAGE(STATUS "setting gcc options: -Wall -Werror -Wno-deprecated -pedantic-errors")
#     # NON-optimized build:
#     # add_definitions( -Wall  -Wno-deprecated -Werror -pedantic-errors)
#     add_definitions(-fPIC)
# endif (CMAKE_BUILD_TOOL MATCHES "make")
#
# option(BUILD_TYPE
#   "Build type: Release=ON/Debug=OFF  " ON)
#
# if (BUILD_TYPE)
#     MESSAGE(STATUS " CMAKE_BUILD_TYPE = Release")
#     set(CMAKE_BUILD_TYPE Release)
# endif(BUILD_TYPE)
#
# if (NOT BUILD_TYPE)
#     MESSAGE(STATUS " CMAKE_BUILD_TYPE = Debug")
#     set(CMAKE_BUILD_TYPE Debug)
# endif(NOT BUILD_TYPE)
#
# # this figures out the Python include directories and adds them to the
# # header file search path
# execute_process(
#     COMMAND python-config --includes
#     COMMAND sed -r "s/-I//g; s/ +/;/g"
#     COMMAND tr -d '\n'
#     OUTPUT_VARIABLE Python_Includes
# )
# include_directories(${Python_Includes})
# include_directories(${CMAKE_CURRENT_BINARY_DIR})

# this defines the source-files
set(AREA_SRC
    Arc.cpp
    Arc.h
    Area.cpp
    Area.h
    AreaClipper.cpp
    AreaDxf.cpp
    AreaDxf.h
    AreaOrderer.cpp
    AreaOrderer.h
    AreaPocket.cpp
    Circle.cpp
    Circle.h
    Curve.cpp
    Curve.h
    dxf.cpp
    dxf.h
    Point.h
    PythonStuff.cpp
    PythonStuff.h
    clipper/clipper.cpp
    clipper/clipper.hpp
    kurve/Construction.cpp
    kurve/Finite.cpp
    kurve/geometry.h
    kurve/kurve.cpp
    kurve/Matrix.cpp
    kurve/offset.cpp
)

# include directories
include_directories(
    kurve/
    clipper/
)

# this makes the Python module
add_library(AreaClipper SHARED ${AREA_SRC})
target_link_libraries(AreaClipper ${AreaClipper_LIBS} )
# set_target_properties(libarea-clipper PROPERTIES PREFIX "")


#I want to install this locally in /Mod/CNC/libs so the next
#section of code isn't needed

#
# this figures out where to install the Python modules
#
#execute_process(
#    COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()"
#    OUTPUT_VARIABLE Python_site_packages
#    OUTPUT_STRIP_TRAILING_WHITESPACE
#)
#MESSAGE(STATUS "Python libraries will be installed to: " ${Python_site_packages})

## this installs the python library
#install(
#	TARGETS area
#	LIBRARY DESTINATION ${Python_site_packages}
#)

if(MSVC)
    set_target_properties(AreaClipper PROPERTIES SUFFIX ".pyd")
    set_target_properties(AreaClipper PROPERTIES DEBUG_OUTPUT_NAME "AreaClipper_d")
    set_target_properties(AreaClipper PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Mod/Cam)
    set_target_properties(AreaClipper PROPERTIES PREFIX "../")
elseif(MINGW)
    set_target_properties(AreaClipper PROPERTIES SUFFIX ".pyd")
    set_target_properties(AreaClipper PROPERTIES DEBUG_OUTPUT_NAME "AreaClipper_d")
    set_target_properties(AreaClipper PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Mod/Cam)
    set_target_properties(AreaClipper PROPERTIES PREFIX "")
else(MSVC)
    set_target_properties(AreaClipper PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Mod/Cam)
    set_target_properties(AreaClipper PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(AreaClipper PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
endif(MSVC)


INSTALL(TARGETS AreaClipper DESTINATION lib)
