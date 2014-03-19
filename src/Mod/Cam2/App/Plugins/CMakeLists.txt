
# make it easier to get to the TPG files (#include <TPG/...>)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/Mod/Cam2/App)

# Add Tpg librarys here
add_subdirectory(TestPlugin)

# Tpg library that parses and post-processes
# the output of another Tpg library
add_subdirectory(InverseKinematicPlugin)