/***************************************************************************
 *   Copyright (c) 2012 Luke Parry    (l.parry@warwick.ac.uk)              *
 *   Copyright (c) 2013 Andrew Robinson <andrewjrobinson@gmail.com>        *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#include <PreCompiled.h>
#ifndef _PreComp_
# include <Python.h>
#endif

#include <Base/Console.h>
#include <Base/Interpreter.h>

#include "Features/CamFeature.h"
#include "Features/CamSettingsableFeature.h"
#include "Features/MachineFeature.h"
#include "Features/MachineProgramFeature.h"
#include "Features/ToolFeature.h"
#include "Features/ToolPathFeature.h"
#include "Features/TPGFeature.h"

#include "TPG/TPGFactory.h"
#include "TPG/TPG.h"
#include "TPG/PyTPGFactory.h"

#include "TPG/PyTPGSettings.h"


// C++ TPG Plugins
// #include "Plugins/MyPlugin.h"

extern struct PyMethodDef Cam_methods[];

PyDoc_STRVAR(module_Cam_doc,
"This module is the CAM module.");


#include "TPG/PyToolPath.h"
#include "TPG/PyTPGSettings.h"

extern "C" void CamExport initCam();	// Forward declaration.

/* Python entry */
extern "C" {
void CamExport initCam()
{
	/*
	#ifdef CAM_BINARY_DIR
		try {
			QString cam_binary_dir = QString::fromAscii(CAM_BINARY_DIR);
			if (cam_binary_dir.endsWith(QString::fromAscii("\\\""))) cam_binary_dir.remove(cam_binary_dir.size()-2, 2);
			if (cam_binary_dir.startsWith(QString::fromAscii("\\\""))) cam_binary_dir.remove(0, 2);

			QString path = cam_binary_dir + QString::fromAscii("/Mod/PyCam/PyPostProcessor/PostProcessor");
			printf("Adding python path %s\n", path.toAscii().constData());
			Base::Interpreter().addPythonPath(path.toAscii().constData());
		}
		catch(const Base::Exception& e) {
			PyErr_SetString(PyExc_ImportError, e.what());
			return;
		}
	#endif
	*/

    // load dependent module
    try {
		printf("Issuing Python command 'import Part'\n");
        Base::Interpreter().runString("import Part");
    }
    catch(const Base::Exception& e) {
        PyErr_SetString(PyExc_ImportError, e.what());
        return;
    }

    PyObject* camModule = Py_InitModule3("Cam", Cam_methods, module_Cam_doc);   // mod name, table ptr

    // Add Types to module
    Py_INCREF(PyToolPath_Type());
    PyModule_AddObject(camModule, "ToolPath", (PyObject *)PyToolPath_Type());
    if (PyType_Ready(PyToolPath_Type()) < 0)
        return;

    Py_INCREF(PySettingsDefinition_Type());
    PyModule_AddObject(camModule, "SettingsDefinition", (PyObject *)PySettingsDefinition_Type());
    if (PyType_Ready(PySettingsDefinition_Type()) < 0)
        return;

    Py_INCREF(PyTPGSettings_Type());
    PyModule_AddObject(camModule, "TPGSettings", (PyObject *)PyTPGSettings_Type());
    if (PyType_Ready(PyTPGSettings_Type()) < 0)
        return;

    // Add the PyCam module to the Cam Module
    PyObject* pyCamMod = PyImport_ImportModule("PyCam");
    if (pyCamMod != NULL)
    {
        PyModule_AddObject(camModule, "PyCam", pyCamMod);
        Cam::PyTPGFactory().loadCallbackFromModule(pyCamMod);
    	Py_DecRef(pyCamMod);
    }

    // NOTE: To finish the initialization of our own type objects we must
    // call PyType_Ready, otherwise we run into a segmentation fault, later on.
    // This function is responsible for adding inherited slots from a type's base class.

    Cam::CamFeature             ::init();
    Cam::CamSettingsableFeature ::init();
    Cam::MachineFeature         ::init();
    Cam::MachineProgramFeature  ::init();
    Cam::Settings               ::init();
    Cam::ToolFeature            ::init();
    Cam::ToolPathFeature        ::init();
    Cam::TPGFeature             ::init();

    // Perform initial scan to load all TPGDescriptors in the factory to ensure that documents can load these
    Cam::TPGFactory().scanPlugins();
    Base::Console().Log("Loading CAM module... done\n");
	printf("Loading CAM module... done\n");

}

} // extern "C"
