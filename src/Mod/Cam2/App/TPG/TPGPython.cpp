/***************************************************************************
 *   Copyright (c) 2012 Luke Parry      (l.parry@warwick.ac.uk)            *
 *                 2012 Andrew Robinson (andrewjrobinson@gmail.com)        *
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
#endif

#include <QString>

#include <Base/Console.h>

#include "../Features/TPGFeature.h"
#include "TPGPython.h"
#include "PyToolPath.h"
#include "PyTPGSettings.h"

using namespace Cam;

// const char* ts(QString str)
// {
//     return str.toAscii().constData();
// }
//
// const char* ts(QString *str)
// {
//     if (str != NULL)
//         return str->toAscii().constData();
//     return "NULL";
// }

TPGPython::TPGPython(PyObject *cls)
{
	//TODO: check the cls is an instance of PyTPG (somehow, possibly checking for the presence of api methods)
	Py_XINCREF(cls);
	this->cls = cls;
	this->inst = NULL;
}

TPGPython::~TPGPython()
{
	Py_XDECREF(this->cls);
}

PyObject *TPGPython::QStringToPythonUC(const QString &str)
{
	//TODO: check this works properly, it appears to work with the UCS4 characters I tested
	return PyString_FromString((const char*) str.toStdString().c_str());
}

QString TPGPython::PythonUCToQString(PyObject *obj)
{
	Py_UNICODE *pid = PyUnicode_AsUnicode(obj);

	if (pid != NULL)
		return QString::fromUcs4((const uint *) pid); //TODO: this should have a check for older pythons that use Ucs2)
	if (PyString_Check(obj))
		return QString::fromLatin1(PyString_AS_STRING(obj));
	return QString();
}


PyObject *TPGPython::QStringListToPythonUCList(const QStringList &string_list)
{
	PyObject *py_string_list = PyList_New(string_list.size());
	for (QStringList::size_type i=0; i<string_list.size(); i++)
	{
		PyList_SET_ITEM(py_string_list, i, PyString_FromString(string_list.at(i).toAscii().constData()));
	}
	return(py_string_list);
}

QStringList TPGPython::PythonUCListToQStringList(PyObject *obj)
{
	QStringList string_list;
	for (Py_ssize_t i=0; i<PyList_Size(obj); i++)
	{
		PyObject *entry = PyList_GetItem(obj, i);
		string_list.append(PythonUCToQString(PyList_GetItem(obj, i)));
	}
	return(string_list);
}


/**
 * Creates an instance of cls and stores it in inst if it doesn't exist
 */
PyObject *TPGPython::getInst(void)
{
	if (inst == NULL)
	{
		printf("Creating Python Instance\n");
        PyGILState_STATE state = PyGILState_Ensure();
		PyObject *result = PyObject_CallObject(cls, NULL);

		if (result != NULL)
			inst = result;
        PyGILState_Release(state);
	}
	return inst;
}

// TPG API methods.  Used by the CAM workbench to run the TPG

QString TPGPython::getId()
{
	if (this->id.size() == 0 && this->cls != NULL)
	{
        PyGILState_STATE state = PyGILState_Ensure();
		PyObject *result = PyObject_CallMethod(this->cls, "getId", NULL);
		if (result != NULL)
		{
			this->id = PythonUCToQString(result);
			Py_DecRef(result);
		}
        PyGILState_Release(state);
	}
	return this->id;
}
QString TPGPython::getName()
{
	if (this->cls != NULL)
	{
        PyGILState_STATE state = PyGILState_Ensure();
		PyObject * result = PyObject_CallMethod(this->cls, "getName", NULL);
		if (result != NULL)
		{
			this->name = PythonUCToQString(result);
			Py_DecRef(result);
		}
        PyGILState_Release(state);
	}
	return this->name;
}
QString TPGPython::getDescription()
{
	if (this->cls != NULL)
	{
        PyGILState_STATE state = PyGILState_Ensure();
		PyObject * result = PyObject_CallMethod(this->cls, "getDescription",
				NULL);
		if (result != NULL)
		{
			this->description = PythonUCToQString(result);
			//			printf("Description: %s\n", ts(description));
			Py_DecRef(result);
		}
        PyGILState_Release(state);
	}
	return this->description;
}

/**
 * Returns a list of action names supported by this TPG.
 *
 * Note: if this TPG only does one thing then just return a list containing 'default'
 */
std::vector<QString> &TPGPython::getActions()
{
	PyObject *inst = getInst();
	if (inst != NULL)
	{
        PyGILState_STATE state = PyGILState_Ensure();
		PyObject *result = PyObject_CallMethod(inst, "getActions", NULL);
		if (result != NULL)
		{
			if (PyList_Check(result))
			{
				int len = PyList_Size(result);
				for (int i = 0; i < len; i++)
				{
					PyObject *item = PyList_GetItem(result, i);
					this->actions.push_back(PythonUCToQString(item));
				}
			}
			Py_DecRef(result);
		}
        PyGILState_Release(state);
	}
	return this->actions;
}

/**
 * Get the settings for this TPG
 * Note: the returned pointer is a deep copy so do as you like to it.
 * You're the owner release it when your done.
 *
 * TODO: load the options for each setting
 */
Settings::TPGSettings *TPGPython::getSettingDefinitions()
{
	if (settings == NULL) {
		PyObject *inst = getInst();
		if (inst != NULL) {
			// Run the method
	        PyGILState_STATE state = PyGILState_Ensure();
			PyObject *settingsObj = PyObject_CallMethod(inst, "getSettingDefinitions", NULL);
			if (settingsObj != NULL) {
				Base::Console().Log("Called getSettingDefinitions() ok.\n");

				if (PyCamTPGSettings_Check(settingsObj)) {
					Base::Console().Log("Got settings object.\n");
					cam_PyTPGSettings *pyTPGSettings = (cam_PyTPGSettings*) settingsObj;
					settings = pyTPGSettings->settings->clone();
				}
				else
					Base::Console().Warning("Value returned from PyTPG.getSettingDefinitions() is not a Cam.TPGSettings object.\n");

				Py_DecRef(settingsObj);

//				// Extract the Each action from the Dictionary
//				if (PyDict_Check(settingsDict)) {
//					PyObject *settingsDictKeys = PyDict_Keys(settingsDict);
//					if (PyList_Check(settingsDictKeys)) {
//						int settingsDictKeysLen = PyList_Size(settingsDictKeys);
//						for (int k = 0; k < settingsDictKeysLen; k++) {
//							PyObject *settingsDictKey = PyList_GetItem(settingsDictKeys, k);
//
//							// Extract settings from list
//							PyObject *settingsDictValue = PyDict_GetItem(settingsDict, settingsDictKey);
//							if (PyList_Check(settingsDictValue)) {
//								settings = new TPGSettings();
//								int len = PyList_Size(settingsDictValue);
//								for (int i = 0; i < len; i++) {
//
//									// Extract details of each setting
//									PyObject *item = PyList_GetItem(settingsDictValue, i);
//									if (PyTuple_Check(item)) {
//										char *name;
//										char *label;
//										char *type;
//										char *defaultvalue;
//										char *units;
//										char *helptext;
//
//										if (PyArg_ParseTuple(item, "zzzzzz", &name, &label, &type,
//												&defaultvalue, &units, &helptext)){
//											QString qaction = QString::fromAscii(PyString_AsString(settingsDictKey));
//											settings->addSettingDefinition(qaction,
//													new TPGSettingDefinition(name, label, type, defaultvalue,
//															units, helptext));
//										}
//										else
//											//TODO: make this more informative
//											Base::Console().Warning("Setting tuple is meant to contain 6 items!\n");
//									}
//									else
//										//TODO: make this more informative
//										Base::Console().Warning("Not a Tuple!\n");
//								}
//							}
//							else
//								//TODO: make this more informative
//								Base::Console().Warning("Value for Action is meant to be a list.  Ignoring this Action.\n");
//						}
//					}
//					else
//						//TODO: make this more informative
//						Base::Console().Warning("Dictionary key list not a list.  This shouldn't happen.\n");
//				}
//				else
//					//TODO: make this more informative
//					Base::Console().Warning("Settings needs to be a dictionary.\n");
//				Py_DecRef(settingsDict);
			}
			else
				Base::Console().Warning("Unable to execute PyTPG.getSettingDefinitions() method.\n");
	        PyGILState_Release(state);
		}
		else
			//TODO: make this more informative
			Base::Console().Warning("Unable to create an instance of TPG's Python Class.\n");
	}

	if (settings != NULL)
		return settings->clone();

	Base::Console().Warning("Unable to load the settings object.\n");
	return NULL;
}

/**
 * Run the TPG to generate the ToolPath code.
 *
 * Note: the return will change once the TP Language has been set in store
 */
void TPGPython::run(Settings::TPGSettings *settings, ToolPath *toolpath, QString action)
{
	PyObject *inst = getInst();
	if (inst != NULL)
	{
        PyGILState_STATE state = PyGILState_Ensure();

        // format other args
        PyObject *toolpathArg = PyToolPath_New(toolpath);
        PyObject *settingsArg = PyTPGSettings_New(settings);
		PyObject *actionArg = QStringToPythonUC(action);

		// run python method
		PyObject *result = PyObject_CallMethod(inst, "run", "(OOO)", settingsArg, toolpathArg, actionArg);
		if (result != NULL)
		{
			Py_DecRef(result);
			printf("pyAction: %s Successful\n", action.toAscii().constData());
		}
		else
			printf("Failed to run pyAction: %s\n",
					action.toAscii().constData());

		// cleanup
        Py_XDECREF(toolpathArg);
        Py_XDECREF(settingsArg);
		Py_XDECREF(actionArg);
        PyGILState_Release(state);
	}
	//TODO: make error
}

///**
// * Returns the toolpath from the last (or current) tool-path run
// */
//ToolPath *TPGPython::getToolPath() {
//    PyObject *inst = getInst();
//    ToolPath *res = NULL;
//    if (inst != NULL) {
//        PyGILState_STATE state = PyGILState_Ensure();
//        PyObject *result = PyObject_CallMethod(inst, "getToolPath", "");
//        if (result != NULL) {
//            if (typeid(result) == typeid(cam_PyToolPath)) {
//                res = ((cam_PyToolPath*) result)->tp;
//            }
//            Py_DecRef(result);
//        }
//        PyGILState_Release(state);
//    }
//    return res;
//}
