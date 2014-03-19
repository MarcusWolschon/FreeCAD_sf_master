
# Set the name of the plugin (excluding the .so)
# While not required this should match the name of the directory we are infor consistency 
SET(TPG_TARGET InverseKinematicPlugin) 

# Add a CAM TOOL PATH GENERATOR PLUGIN macro so that we can set the dllimport/dllexport linkages correctly.
add_definitions(-DCAM_TPG_PLUGIN)

include_directories(
	${CMAKE_SOURCE_DIR}/src/Mod/Cam2/App
	${CMAKE_SOURCE_DIR}/src/Mod/Cam2/App/Graphics
	)

# Add your .h and .cpp files here
SET(Tpg_SRCS
	CppInverseKinematicTPG.cpp
	CppInverseKinematicTPG.h

	../../TPG/CppTPGDescriptor.h
	../../TPG/CppTPGDescriptorWrapper.h
	../../TPG/CppTPGPlugin.h
	../../TPG/TPGDescriptor.h
	../../TPG/TPGDescriptorCollection.h
	../MyPlugin.cpp
	../MyPlugin.h
)