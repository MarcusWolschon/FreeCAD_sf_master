if(MSVC)
    add_definitions(-DFCAppCamGui -DHAVE_ACOSH -DHAVE_ASINH -DHAVE_ATANH)
else(MSVC)
    add_definitions(-DFCAppCamGui -DHAVE_LIMITS_H -DHAVE_CONFIG_H)
endif(MSVC)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/Mod/Cam2/Gui
    ${CMAKE_SOURCE_DIR}/src/Mod/Cam2/App/Graphics
    ${CMAKE_SOURCE_DIR}/src/Mod/Cam2/App/Libs/libarea-clipper
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Boost_INCLUDE_DIRS}
    ${COIN3D_INCLUDE_DIR}
    ${OCC_INCLUDE_DIR}
    ${PYTHON_INCLUDE_PATH}
    ${SOQT_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
)

if(MSVC)
    set(CamGui_LIBS
        Cam
        Mesh
        PartGui
        FreeCADGui
    )
else(MSVC)
    set(CamGui_LIBS
        Cam
        PartGui
        FreeCADGui
    )
endif(MSVC)

qt4_add_resources(CamGui_SRCS Resources/Cam.qrc)

set(CamGui_MOC_HDRS
    TaskDialog/TaskDlgEditCamFeature.h
    # TaskDialog/TaskDlgEditCamPartsList.h
    # TaskDialog/TaskDlgEditStockGeometry.h
    TaskDialog/TaskDlgEditTPGFeature.h
    
    CamProjectDockWindow.h
    CamProjectDockWindowComponents.h
    MachineProgramDockWindow.h
    TPGLibraryDockWindow.h
    TPGListModel.h
    ToolpathDockWindow.h
    UIManager.h
)

set (CamGui_MOC_SRCS
	../App/CamManager.cpp
)

fc_wrap_cpp(CamGui_MOC_SRCS ${CamGui_MOC_HDRS})

set(CamGui_UIC_SRCS
    CamProjectDockWindow.ui
    MachineProgramDockWindow.ui
    TPGLibraryDockWindow.ui
    ToolpathDockWindow.ui
)
qt4_wrap_ui(CamGui_UIC_HDRS ${CamGui_UIC_SRCS})

SET(CamGui_SRCS
    ${CamGui_SRCS}
    ${CamGui_UIC_HDRS}
    Resources/Cam.qrc
    AppCamGui.cpp
    Command.cpp
    PreCompiled.cpp
    PreCompiled.h
    TaskDialog/TaskDlgEditCamFeature.cpp
    TaskDialog/TaskDlgEditCamFeature.h
    TaskDialog/TaskDlgEditTPGFeature.cpp
    TaskDialog/TaskDlgEditTPGFeature.h
    Workbench.cpp
    Workbench.h
    ViewProviderCamFeature.cpp
    ViewProviderCamFeature.h
    ViewProviderTPGFeature.cpp
    ViewProviderTPGFeature.h
    ViewProviderToolFeature.cpp
    ViewProviderToolFeature.h
    ViewProviderToolPathFeature.cpp
    ViewProviderToolPathFeature.h
    ViewProviderMachineFeature.cpp
    ViewProviderMachineFeature.h
    ViewProviderMachineProgramFeature.cpp
    ViewProviderMachineProgramFeature.h
    CamProjectDockWindow.cpp
    CamProjectDockWindow.h
    CamProjectDockWindowComponents.cpp
    CamProjectDockWindowComponents.h
	MachineProgramDockWindow.cpp
	MachineProgramDockWindow.h
    TPGLibraryDockWindow.cpp
    TPGLibraryDockWindow.h
    ToolpathDockWindow.cpp
    ToolpathDockWindow.h
    UIManager.cpp
    UIManager.h
    TPGListModel.cpp
    TPGListModel.h

    ../App/TPG/TPGDescriptor.h
    ../App/TPG/TPGDescriptorCollection.h
    ../App/TPG/TPGSettings.h
    ../App/CamManager.h
)

add_library(CamGui SHARED ${CamGui_SRCS})
target_link_libraries(CamGui ${CamGui_LIBS})

fc_target_copy_resource(CamGui
    ${CMAKE_SOURCE_DIR}/src/Mod/Cam2
    ${CMAKE_BINARY_DIR}/Mod/Cam
    InitGui.py)

if(MSVC)
    set_target_properties(CamGui PROPERTIES SUFFIX ".pyd")
    set_target_properties(CamGui PROPERTIES DEBUG_OUTPUT_NAME "CamGui_d")
    set_target_properties(CamGui PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Mod/Cam)
    set_target_properties(CamGui PROPERTIES PREFIX "../")
elseif(MINGW)
    set_target_properties(CamGui PROPERTIES SUFFIX ".pyd")
    set_target_properties(CamGui PROPERTIES DEBUG_OUTPUT_NAME "CamGui_d")
    set_target_properties(CamGui PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Mod/Cam)
    set_target_properties(CamGui PROPERTIES PREFIX "")
else(MSVC)
    set_target_properties(CamGui PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Mod/Cam)
    set_target_properties(CamGui PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(CamGui PROPERTIES INSTALL_RPATH ${INSTALL_RPATH})
endif(MSVC)

INSTALL(TARGETS CamGui DESTINATION lib)