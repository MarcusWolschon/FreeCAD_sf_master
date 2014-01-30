if(MSVC)
    add_definitions(-DFCAppCam -DHAVE_ACOSH -DHAVE_ASINH -DHAVE_ATANH)
else(MSVC)
    add_definitions(-DHAVE_LIMITS_H -DHAVE_CONFIG_H)
endif(MSVC)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/Mod/Cam2/App
    ${CMAKE_SOURCE_DIR}/src/Mod/Cam2/App/Features
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Boost_INCLUDE_DIRS}
    ${OCC_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${PYTHON_INCLUDE_PATH}
    ${XERCESC_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/Mod/Cam2/App/Libs/libarea-clipper
)
link_directories(${OCC_LIBRARY_DIR})

set(Cam_LIBS
    Part
    AreaClipper
    ${OCC_LIBRARIES}
    FreeCADApp
)

set(Cam_MOC_HDRS
    CamManager.h
)
fc_wrap_cpp(Cam_MOC_SRCS ${Cam_MOC_HDRS})

add_subdirectory(Libs)
add_subdirectory(Plugins)

# generate_from_xml(CamPy)

SET(Graphics_SRCS
    Graphics/CDouble.h
    Graphics/Paths.h
    Graphics/Paths.cpp
)
SOURCE_GROUP("Graphics" FILES ${Graphics_SRCS})

SET(Features_SRCS
    Features/CamFeature.cpp
    Features/CamFeature.h
    Features/CamSettingsableFeature.cpp
    Features/CamSettingsableFeature.h
    Features/Feature.cpp
    Features/Feature.h
    Features/MachineFeature.cpp
    Features/MachineFeature.h
    Features/MachineProgramFeature.cpp
    Features/MachineProgramFeature.h
    Features/ToolFeature.cpp
    Features/ToolFeature.h
    Features/ToolPathFeature.cpp
    Features/ToolPathFeature.h
    Features/TPGFeature.cpp
    Features/TPGFeature.h
)
SOURCE_GROUP("Features" FILES ${Features_SRCS})

#Classes related to Tool Path Generation
SET(TPG_SRCS
    TPG/TPG.cpp
    TPG/TPG.h
    TPG/TPGCache.cpp
    TPG/TPGCache.h
    TPG/TPGDescriptor.cpp
    TPG/TPGDescriptor.h
    TPG/TPGDescriptorCollection.cpp
    TPG/TPGDescriptorCollection.h
    TPG/TPGFactory.cpp
    TPG/TPGFactory.h
    TPG/TPGLib.cpp
    TPG/TPGLib.cpp
    TPG/TPGProcess.cpp
    TPG/TPGProcess.h
    TPG/TPGPython.cpp
    TPG/TPGPython.h
    TPG/TPGSettings.cpp
    TPG/TPGSettings.h
    TPG/ToolPath.cpp
    TPG/ToolPath.h
    
    TPG/PyTPGFactory.cpp
    TPG/PyTPGFactory.h
    TPG/PyTPGDescriptor.cpp
    TPG/PyTPGDescriptor.h
    TPG/PyTPGSettings.cpp
    TPG/PyTPGSettings.h
    TPG/PyToolPath.cpp
    TPG/PyToolPath.h
    
    TPG/CppTPG.h
    TPG/CppTPG.cpp
    TPG/CppTPGDescriptor.cpp
    TPG/CppTPGDescriptor.h
    TPG/CppTPGDescriptorWrapper.cpp
    TPG/CppTPGDescriptorWrapper.h
    TPG/CppTPGFactory.cpp
    TPG/CppTPGFactory.h
    TPG/CppTPGPlugin.cpp
    TPG/CppTPGPlugin.h
    # TPG/CppTPGWrapper.cpp
    # TPG/CppTPGWrapper.h
)
SOURCE_GROUP("TPG" FILES ${TPG_SRCS})

SET(PLUGINS_SRCS
	Plugins/MyPlugin.cpp
	Plugins/MyPlugin.h
)

SOURCE_GROUP("Plugins" FILES ${PLUGINS_SRCS})

# SET(Python_SRCS
# )
# SOURCE_GROUP("Python" FILES ${Python_SRCS})

SET(CamModule_SRCS
    AppCam.cpp
    AppCamPy.cpp
    LinuxCNC.cpp
    LinuxCNC.h
    GCode.cpp
    GCode.h
    MachineSettings.cpp
    MachineSettings.h
    PreCompiled.cpp
    PreCompiled.h
    Support.cpp
    Support.h
    ItemList.cpp
    ItemList.h
    MachineProgram.cpp
    MachineProgram.h
    PostProcessor.cpp
    PostProcessor.h
    CamManager.cpp
    CamManager.h
)
SOURCE_GROUP("Module" FILES ${CamModule_SRCS})

SET(Cam_SRCS
    ${TPG_SRCS}
    ${CamModule_SRCS}
    ${Features_SRCS}
    ${Graphics_SRCS}
    ${Python_SRCS}
    ${PLUGINS_SRCS}
)

SET(Cam_Scripts
    Init.py
    InitGui.py
)

SET(TPG_Scripts
    __init__.py
    PyCam.py
    PyTPG/__init__.py

    ### Copy Python TPG's to the distribution directory ###
    # Add yours here #
    
    # ExampleTPG module
    PyTPG/ExampleTPG/__init__.py
    PyTPG/ExampleTPG/ExampleTPG.py
)

SOURCE_GROUP("" FILES ${TPG_Scripts})
ADD_CUSTOM_TARGET(TPG_Scripts ALL
    SOURCES ${TPG_Scripts}
)
fc_copy_sources("Cam" "${CMAKE_BINARY_DIR}/Mod/PyCam" ${TPG_Scripts}) #TODO: sort out the C++/Python conflicting package names so this can be moved back inside the Cam directory

SET(PostProc_Scripts
    PyPostProcessor/__init__.py
    PyPostProcessor/PostProcessorAPI.py
    PyPostProcessor/TestPostProcessor.py
    PyPostProcessor/PostProcessor/__init__.py
    PyPostProcessor/PostProcessor/nc.py
    PyPostProcessor/PostProcessor/format.py
    PyPostProcessor/PostProcessor/iso.py
    PyPostProcessor/PostProcessor/linuxcnc.py
    PyPostProcessor/PostProcessor/mach3.py
    PyPostProcessor/PostProcessor/hm50.py

	## TODO: add core postprocessor files

    ### Copy PostProcessors to the distribution directory ###
    # Add yours here #
    # e.g. PyPostProcessor/PostProcessor/MyPostProcessor.py
)

SOURCE_GROUP("" FILES ${PostProc_Scripts})
ADD_CUSTOM_TARGET(PostProc_Scripts ALL
    SOURCES ${PostProc_Scripts}
)
fc_copy_sources("Cam" "${CMAKE_BINARY_DIR}/Mod/PyCam" ${PostProc_Scripts}) #TODO: sort out the C++/Python conflicting package names so this can be moved back inside the Cam directory

install(
    FILES
        ${TPG_Scripts}
    DESTINATION
        Mod/PyCam
)

add_library(Cam SHARED ${Cam_SRCS})
target_link_libraries(Cam ${Cam_LIBS})

fc_target_copy_resource(Cam
    ${CMAKE_SOURCE_DIR}/src/Mod/Cam2
    ${CMAKE_BINARY_DIR}/Mod/Cam
    ${Cam_Scripts})

if(MSVC)
    set_target_properties(Cam PROPERTIES SUFFIX ".pyd")
    set_target_properties(Cam PROPERTIES DEBUG_OUTPUT_NAME "Cam_d")
    set_target_properties(Cam PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Mod/Cam)
    set_target_properties(Cam PROPERTIES PREFIX "../")

elseif(MINGW)
    set_target_properties(Cam PROPERTIES SUFFIX ".pyd")
    set_target_properties(Cam PROPERTIES DEBUG_OUTPUT_NAME "Cam_d")
    set_target_properties(Cam PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Mod/Cam)
    set_target_properties(Cam PROPERTIES PREFIX "")
else(MSVC)
    set_target_properties(Cam PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Mod/Cam)
    set_target_properties(Cam PROPERTIES PREFIX "")
    set_target_properties(Cam PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
endif(MSVC)

add_definitions(-DCAM_BINARY_DIR="${CMAKE_BINARY_DIR}")

INSTALL(TARGETS Cam DESTINATION lib)
