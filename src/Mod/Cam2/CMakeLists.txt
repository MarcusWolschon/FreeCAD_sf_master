add_subdirectory(App)
if(FREECAD_BUILD_GUI)
    add_subdirectory(Gui)
endif(FREECAD_BUILD_GUI)

INSTALL(
    FILES
        Init.py
        InitGui.py
    DESTINATION
        Mod/Cam
)

INSTALL(
    DIRECTORY
      Plugins
    DESTINATION
        ${CMAKE_INSTALL_DATADIR}/Mod/Cam
    PATTERN "Makefile*" EXCLUDE
)

