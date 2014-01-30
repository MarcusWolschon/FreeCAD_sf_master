
# Set the name of the plugin (excluding the .so)
# While not required this should match the name of the directory we are infor consistency 
SET(TPG_TARGET TestPlugin) 

# Add a CAM TOOL PATH GENERATOR PLUGIN macro so that we can set the dllimport/dllexport linkages correctly.
add_definitions(-DCAM_TPG_PLUGIN)

include_directories(
	${CMAKE_SOURCE_DIR}/src/Mod/Cam2/App
	${CMAKE_SOURCE_DIR}/src/Mod/Cam2/App/Graphics
	)

# Add your .h and .cpp files here
SET(Tpg_SRCS
	CppExampleTPG.cpp
	CppExampleTPG.h

	../../TPG/CppTPGDescriptor.h
	../../TPG/CppTPGDescriptorWrapper.h
	../../TPG/CppTPGPlugin.h
	../../TPG/TPGDescriptor.h
	../../TPG/TPGDescriptorCollection.h
	../MyPlugin.cpp
	../MyPlugin.h
)

# Add any libs that are needed
SET(Tpg_LIBS
	Cam
)

SOURCE_GROUP("TPG" FILES ${Tpg_SRCS})

ADD_LIBRARY(${TPG_TARGET} SHARED ${Tpg_SRCS})
TARGET_LINK_LIBRARIES(${TPG_TARGET} ${Tpg_LIBS})

if(MSVC)
	set_target_properties(${TPG_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Mod/Cam/CppTPG)
	set_target_properties(${TPG_TARGET} PROPERTIES PREFIX "../")	# Workaround the Debug/ and Release/ systems for Visual Studio
endif(MSVC)

set_target_properties(${TPG_TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Mod/Cam/CppTPG)

if (MSVC)
	INSTALL(TARGETS ${TPG_TARGET} 
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib )
else(MSVC)
	INSTALL(TARGETS ${TPG_TARGET} DESTINATION lib)
endif(MSVC)
