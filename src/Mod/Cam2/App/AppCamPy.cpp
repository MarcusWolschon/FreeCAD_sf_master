/***************************************************************************
 *   Copyright (c) 2008 J�rgen Riegel (juergen.riegel@web.de)              *
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
# include <BRepPrimAPI_MakeBox.hxx>
# include <TopoDS_Face.hxx>
# include <Geom_Plane.hxx>
# include <Handle_Geom_Plane.hxx>
#endif

//#include <Python.h>
#include <iostream>

#include <Base/Console.h>
#include <Base/PyObjectBase.h>
#include <Base/Exception.h>
#include <Base/FileInfo.h>
#include <App/Application.h>
#include <App/Document.h>

// Things from the part module
#include <Mod/Part/App/TopoShape.h>
#include <Mod/Part/App/TopoShapePy.h>

#include "TPG/PyTPGFactory.h"
#include "TPG/TPG.h"
#include "Support.h"
#include "CamManager.h"

using Base::Console;
using namespace Part;
using namespace std;

//const char* ts(QString &str)
//{
//	return str.toAscii().constData();
//}
//const char* ts(QString str)
//{
//    return str.toAscii().constData();
//}
//
//const char* ts(QString *str)
//{
//    if (str != NULL)
//        return str->toAscii().constData();
//    return "NULL";
//}

/* module functions */
static PyObject * open(PyObject *self, PyObject *args)
{
    const char* Name;
    if (!PyArg_ParseTuple(args, "s",&Name))
        return NULL;

    PY_TRY {
    } PY_CATCH;
        //Base::Console().Log("Open in Part with %s",Name);
        Base::FileInfo file(Name);

        // extract ending
        if (file.extension() == "")
            Py_Error(PyExc_Exception,"no file ending");

        //if (file.hasExtension("igs") || file.hasExtension("iges")) {
        //    // create new document and add Import feature
        //    App::Document *pcDoc = App::GetApplication().newDocument(file.fileNamePure().c_str());
        //    Part::ImportIges *pcFeature = (Part::ImportIges*) pcDoc->addObject("Part::ImportIges",file.fileNamePure().c_str());
        //    pcFeature->FileName.setValue(Name);
        //    pcDoc->recompute();
        //}
        // else {
            Py_Error(PyExc_Exception,"unknown file ending");
        //}

    Py_Return;
}

/* PyTPG API functions */
static PyObject * _registerPyTPGFactory_(PyObject *self, PyObject *args)
{
	PyObject *obj;
//	PyObject *method;
	if (!PyArg_ParseTuple(args, "O", &obj)) { //, &method
		char *error = new char[120];
		error[0] = 0;
		sprintf(error, "Cam._registerTPGManagerCallback_() expects 1 parameters, but given %i", PyTuple_Size(args));
		PyErr_SetString(PyExc_TypeError, error);
		return NULL;
	}

	// TODO: add this check back in so that it checks the obj is a class instance
//	if (!PyInstance_Check(obj)) {
//		char *error = new char[150];
//		error[0] = 0;
//		sprintf(error, "Cam._registerTPGManagerCallback_(), argument 1 must be an object instance, but given %s", Py_TYPE(obj)->tp_name);
//		PyErr_SetString(PyExc_TypeError, error);
//		return NULL;
//	}

	// add the callback to the API
	Cam::PyTPGFactory().setCallback(obj); //, method
//    Cam::PyTPGAPI().test();
    Py_Return;
}

/**
 *  Updates the UI to show the progress of the PyTPG's processing.
	States:
	- 'UNDEFINED': The default state before anything is done.
	- 'LOADED': Un-used at the moment.
	- 'INITIALISED':  UIManager sets this state just before calling the PyTPG's run method.
	- 'STARTED': PyTPG should set this state as soon as it enters the run method.
	- 'RUNNING': PyTPG should set this state after things are setup and about to start main loop.
	- 'ERROR': If processing fails then set this state to indicate this.  Run method should stop execution soon after this.
	- 'FINISHED': When processing is finished successfully set state to this.  Run method should stop execution soon after this.

	@param state: string, one of the states above.
	@param progress: int, the overall progress (percentage) of running this TPG.  Should be >= 1 and <= 99 as 0 and 100 are used by UIManager

	TODO: move this method to the Python Export of the CamManager
 */
static PyObject *updateProgressPyTPG(PyObject *self, PyObject *args)
{
	char *tpgid;
	char *status;
	int   progress = -1;
//	PyObject *method;
	if (!PyArg_ParseTuple(args, "ss|i", &tpgid, &status, &progress)) {
		char *error = new char[120];
		error[0] = 0;
		sprintf(error, "Cam.updateProgressPyTPG() expects 2 or 3 parameters (str, str, int), but given %i", PyTuple_Size(args));
		PyErr_SetString(PyExc_TypeError, error);
		return NULL;
	}
	Cam::TPG::State state;

	// convert status to enum type
	if (strcmp(status, "RUNNING") == 0)
		state = Cam::TPG::RUNNING;
	else if (strcmp(status, "INITIALISED") == 0)
		state = Cam::TPG::INITIALISED;
	else if (strcmp(status, "STARTED") == 0)
		state = Cam::TPG::STARTED;
	else if (strcmp(status, "FINISHED") == 0)
		state = Cam::TPG::FINISHED;
	else if (strcmp(status, "UNDEFINED") == 0)
		state = Cam::TPG::UNDEFINED;
	else if (strcmp(status, "ERROR") == 0)
		state = Cam::TPG::ERROR;
	else if (strcmp(status, "LOADED") == 0)
		state = Cam::TPG::LOADED;

	Cam::CamManager().updateProgress(QString::fromAscii(tpgid), state, progress);

	Py_Return;
}

static PyObject *test(PyObject *self, PyObject *args)
{
//	Cam::PyTPGManager().test();
    Cam::TPGFactory().scanPlugins();
    Cam::TPGDescriptorCollection *plugins = Cam::TPGFactory().getDescriptors();
	for (::size_t i = 0; i < plugins->size(); i++)
		plugins->at(i)->print();

	if (plugins->size() > 0)
	{
	    Cam::TPGDescriptor* descriptor = plugins->at(0);
        Cam::TPG *tpg = descriptor->make();
        if (tpg != NULL) {
            QString desc = tpg->getDescription();
    //		printf("Found PyTPG: %s [%s] '%s'\n",ts(tpg->getName()),ts(tpg->getId()),ts(desc));
            printf("Found TPG: %s [%s]",ts(tpg->getName()),ts(tpg->getId()));
            printf(" '%s'\n", ts(desc));
            // test the pyTPG API
            vector<QString> actions = tpg->getActions();
			for (::size_t i = 0; i < actions.size(); i++) {
                printf(" - Action: %s\n", ts(actions[i]));
                Cam::Settings::TPGSettings *settings = tpg->getSettingDefinitions();
                settings->print();
                delete settings;
            }
            tpg->release();
        }
        else {
            printf("Unable to make TPG with id: [%s]\n", ts(descriptor->id));
        }
	}
	Py_Return;
}
static PyObject *test1(PyObject *self, PyObject *args)
{

	PyObject *str;
//	PyObject *method;
	if (!PyArg_ParseTuple(args, "O", &str)) {
		char *error = new char[120];
		error[0] = 0;
		sprintf(error, "Cam.test1() expects 1 parameters, but given %i", PyTuple_Size(args));
		PyErr_SetString(PyExc_TypeError, error);
		return NULL;
	}

	if (!Cam::PyTPGFactory().test1(str))
		return NULL;

	Py_Return;
}


/* registration table  */
struct PyMethodDef Cam_methods[] = {
    {"open",                   open,                   METH_VARARGS, "A Copy/Paste remnant :P"},
    {"_registerPyTPGFactory_", _registerPyTPGFactory_, METH_VARARGS, "Do not use; this is an internal method used by the Cam::PyTPGFactory"},
    {"updateProgressPyTPG",    updateProgressPyTPG,    METH_VARARGS, "Do not use; this is an internal method.  Update the processing status of a running PyTPG."},

    {"test",                   test,                   METH_VARARGS, "Perform some testing."},
    {"test1",                  test1,                  METH_VARARGS, "Perform some testing with 1 arg."},

    {NULL, NULL}        /* end of table marker */
};
