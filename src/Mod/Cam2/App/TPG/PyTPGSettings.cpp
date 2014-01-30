/***************************************************************************
 *   Copyright (c) 2012 Andrew Robinson <andrewjrobinson@gmail.com>        *
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

#include "PyTPGSettings.h"


// ----------------------------------------------------------------------------
// ----- TPGSetting -----------------------------------------------------------
// ----------------------------------------------------------------------------

// ----- Forward declarations -----
static void      PySettingsDefinition_dealloc(cam_PySettingsDefinition* self);
static PyObject *PySettingsDefinition_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int       PySettingsDefinition_init(cam_PySettingsDefinition *self, PyObject *args, PyObject *kwds);
static PyObject *PySettingsDefinition_addOption (cam_PySettingsDefinition* self, PyObject* args);


// ----- Data structures instances -----

/**
 * Method table for PyToolPath python type
 */
static PyMethodDef PySettingsDefinition_methods[] = {
    {"addOption", (PyCFunction)PySettingsDefinition_addOption, METH_VARARGS, "Add an option to this setting"},
//    {"getAction", (PyCFunction)PyTPGSettings_getAction, METH_NOARGS, "Get the selected action"},
//    {"setAction", (PyCFunction)PyTPGSettings_setAction, METH_VARARGS, "Set the selected action"},
//    {"getValue", (PyCFunction)PyTPGSettings_getValue, METH_VARARGS, "Get the value of a setting"},
//    {"setValue", (PyCFunction)PyTPGSettings_setValue, METH_VARARGS, "Set the value of a setting"},
//    {"addDefaults", (PyCFunction)PyTPGSettings_addDefaults, METH_NOARGS, "Adds any missing setting values (with their default value)"},
    {NULL}  /* Sentinel */
};

static PyTypeObject PySettingsDefinitionType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Cam.SettingsDefinition",       /*tp_name*/
    sizeof(cam_PySettingsDefinition), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PySettingsDefinition_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,       /*tp_flags*/
    "Storage object for a collection of TPG Settings", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PySettingsDefinition_methods,     /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PySettingsDefinition_init, /* tp_init */
    0,                         /* tp_alloc */
    PySettingsDefinition_new,         /* tp_new */
};


extern PyTypeObject *PySettingsDefinition_Type() {
	return &PySettingsDefinitionType;
}


// ----- Function implementations ----

/**
 * The python deallocator
 */
static void
PySettingsDefinition_dealloc(cam_PySettingsDefinition* self) {
    if (self->setting != NULL) {
        self->setting->release();
        self->setting = NULL;
    }
    self->ob_type->tp_free((PyObject*)self);
}

/**
 * The python allocator
 */
static PyObject *
PySettingsDefinition_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    cam_PyTPGSettings *self;

    self = (cam_PyTPGSettings *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->settings = NULL; //new Cam::Settings::TPGSettings();
    }

    return (PyObject *)self;
}

/**
 * Python initialiser
 *
 * NOTE: The ASCII values available for the TPGSettingDefinition::type are effectively
 *       defined here.  eg: "Cam::Text" converts to Cam::Settings::Definition::SettingType_Text in
 *       this routine.  If values are added to the Cam::Settings::Definition::SettingType enumeration
 *       then the corresponding values MUST be added here as well.
 */
static int
PySettingsDefinition_init(cam_PySettingsDefinition *self, PyObject *args, PyObject *kwds) {

	const char *name;
	const char *label;
	const char *type;
	const char *defaultvalue;
	const char *units;
	const char *helptext;

	//TODO: make sure kwds doesn't have any values
	if (!PyArg_ParseTuple(args, "ssssss", &name, &label, &type, &defaultvalue, &units, &helptext)) {
		PyErr_SetString(PyExc_TypeError, "Cam.TPGSettingDefinition(): expects '<name>, <label>, <type>, <defaultvalue>, <units>, <helptext>' (6 strings)");
		return -1;
	}

	// TODO: Add the ability to create settings of all supported types now that we're not using the Cam::Settings::Definition object directly any more.
	QString qsType = QString::fromAscii(type);
	Cam::Settings::Definition::SettingType data_type = Cam::Settings::Definition::SettingType_Text;

	     if (qsType == QString::fromAscii("Cam::Text")) self->setting = new Cam::Settings::Text(name, label, defaultvalue, units, helptext);
	else if (qsType == QString::fromAscii("Cam::Radio")) self->setting = new Cam::Settings::Radio(name, label, defaultvalue, helptext);
	/*
	else if (qsType == QString::fromAscii("Cam::ObjectNamesForType")) self->setting = new Cam::Settings::ObjectNamesForType(name, label, helptext, delimiters, defaultvalue);
	else if (qsType == QString::fromAscii("Cam::SingleObjectNameForType")) self->setting = new Cam::Settings::SingleObjectNameForType(name, label, helptext, delimiters, defaultvalue);

	

	else if (qsType == QString::fromAscii("Cam::Length")) 
	{
		double def = QString::fromAscii(defaultvalue).toDouble();
		self->setting = new Cam::Settings::Length(name, label, helptext,uble def, Cam::Settings::Definition::Metric);
		self->setting->units = units;
	}
	else if (qsType == QString::fromAscii("Cam::Filename")) data_type = Cam::Settings::Definition::SettingType_Filename;
	else if (qsType == QString::fromAscii("Cam::Directory")) data_type = Cam::Settings::Definition::SettingType_Directory;
	else if (qsType == QString::fromAscii("Cam::Color")) data_type = Cam::Settings::Definition::SettingType_Color;
	else if (qsType == QString::fromAscii("Cam::Colour")) data_type = Cam::Settings::Definition::SettingType_Color;
	else if (qsType == QString::fromAscii("Cam::Integer")) data_type = Cam::Settings::Definition::SettingType_Integer;
	else if (qsType == QString::fromAscii("Cam::Double")) data_type = Cam::Settings::Definition::SettingType_Double;
	else if (qsType == QString::fromAscii("Cam::Float")) data_type = Cam::Settings::Definition::SettingType_Double;

	self->setting = new Cam::Settings::Definition(name, label, data_type, defaultvalue, units, helptext);
	*/

    return 0;
}

/**
 * Add a command to the toolpath
 */
static PyObject *
PySettingsDefinition_addOption (cam_PySettingsDefinition* self, PyObject* args) {

    //Expects (<name>, <label>)

    const char *id;
    const char *label;

    if (!PyArg_ParseTuple(args, "ss", &id, &label)) {
        PyErr_SetString(PyExc_TypeError, "Expects '<id>, <label>' (2 strings)");
        return NULL;
    }

    if (self->setting != NULL) {
    	self->setting->addOption(id, label);
//    	return self; //TODO: return the self pointer for convenience
    }

	PyErr_SetString(PyExc_Exception, "Underlying data-structure not set.");
	return NULL;
}




// ----------------------------------------------------------------------------
// ----- TPGSettings ----------------------------------------------------------
// ----------------------------------------------------------------------------

// ----- Forward declarations -----

static void      PyTPGSettings_dealloc(cam_PyTPGSettings* self);
static PyObject *PyTPGSettings_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int       PyTPGSettings_init(cam_PyTPGSettings *self, PyObject *args, PyObject *kwds);
static PyObject *PyTPGSettings_addSettingDefinition (cam_PyTPGSettings* self, PyObject* args);
static PyObject *PyTPGSettings_getAction (cam_PyTPGSettings* self, PyObject* args);
static PyObject *PyTPGSettings_setAction (cam_PyTPGSettings* self, PyObject* args);
static PyObject *PyTPGSettings_getValue (cam_PyTPGSettings* self, PyObject* args);
static PyObject *PyTPGSettings_setValue (cam_PyTPGSettings* self, PyObject* args);
static PyObject *PyTPGSettings_addDefaults (cam_PyTPGSettings* self, PyObject* args);
static PyObject *PyTPGSettings_getActions (cam_PyTPGSettings* self, PyObject* args);

// ----- Data structures instances -----

static PyMethodDef PyTPGSettings_methods[] = {
    {"addSettingDefinition", (PyCFunction)PyTPGSettings_addSettingDefinition, METH_VARARGS, "Add a setting to this settings collection"},
    {"getAction", (PyCFunction)PyTPGSettings_getAction, METH_NOARGS, "Get the selected action"},
    {"setAction", (PyCFunction)PyTPGSettings_setAction, METH_VARARGS, "Set the selected action"},
    {"getValue", (PyCFunction)PyTPGSettings_getValue, METH_VARARGS, "Get the value of a setting"},
    {"setValue", (PyCFunction)PyTPGSettings_setValue, METH_VARARGS, "Set the value of a setting"},
    {"addDefaults", (PyCFunction)PyTPGSettings_addDefaults, METH_NOARGS, "Adds any missing setting values (with their default value)"},
    {"getActions", (PyCFunction)PyTPGSettings_getActions, METH_NOARGS, "Gets the action names defined in this settings object"},
    {"keys", (PyCFunction)PyTPGSettings_getActions, METH_NOARGS, "Gets the action names defined in this settings object"},
//    {"clone", (PyCFunction)PyTPGSettings_clone, METH_NOARGS, "Clones the settings collection"},
    {NULL}  /* Sentinel */
};

static PyTypeObject PyTPGSettingsType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Cam.TPGSettings",       /*tp_name*/
    sizeof(cam_PyTPGSettings), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyTPGSettings_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,    /*tp_getattro*/ //PyTPGSettings_getattro
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,       /*tp_flags*/
    "Storage object for a collection of TPG Settings", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyTPGSettings_methods,     /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyTPGSettings_init, /* tp_init */
    0,                         /* tp_alloc */
    PyTPGSettings_new,         /* tp_new */
};

extern PyTypeObject *PyTPGSettings_Type() {
	return &PyTPGSettingsType;
}

// ----- Function implementations ----

/**
 * The python deallocator
 */
static void
PyTPGSettings_dealloc(cam_PyTPGSettings* self) {
    if (self->settings != NULL) {
        self->settings->release();
        self->settings = NULL;
    }
    self->ob_type->tp_free((PyObject*)self);
}

/**
 * The python allocator
 */
static PyObject *
PyTPGSettings_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    cam_PyTPGSettings *self;

    self = (cam_PyTPGSettings *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->settings = NULL; //new Cam::Settings::TPGSettings();
    }

    return (PyObject *)self;
}

/**
 * Python initialiser
 */
static int
PyTPGSettings_init(cam_PyTPGSettings *self, PyObject *args, PyObject *kwds) {

	// extract parameter
	PyObject *copy = NULL;
	if (!PyArg_ParseTuple(args, "|O", &copy)) {
		PyErr_SetString(PyExc_TypeError, "Expects an optional TPGSettings object (to clone)");
		return -1;
	}

	if (copy == NULL) {
		self->settings = new Cam::Settings::TPGSettings();
		return 0;
	} else {
		if (PyCamTPGSettings_Check(copy)) {
			cam_PyTPGSettings *pcopy = (cam_PyTPGSettings*) copy;
			if (pcopy->settings != NULL) {
				self->settings = pcopy->settings->clone();
				return 0;
			} else {
				PyErr_SetString(PyExc_Exception, "Underlying data-structure not set.");
				return -1;
			}
		} else {
			PyErr_SetString(PyExc_TypeError, "Expects an optional TPGSettings object (to clone)");
			return -1;
		}
	}
}

///**
// * Python Attribute getter
// */
//static PyObject *
//PyTPGSettings_getattro(cam_PyTPGSettings *self, PyObject *name) {
//
//    // get default attributes (such as methods)
//    PyObject *tmp;
//    if (!(tmp = PyObject_GenericGetAttr((PyObject *) self, name))) {
//        if (!PyErr_ExceptionMatches(PyExc_AttributeError))
//            return NULL;
//        PyErr_Clear();
//    } else
//        return tmp;
//
//    // otherwise try my own attributes
////    char* attrname = PyString_AsString(name);
////    QString *tmpstr = self->settings->getSettings()(attrname);
////    if (tmpstr != NULL)
////        return PyString_FromString(((const char*)(tmpstr->toStdString().c_str())));
//    Py_RETURN_NONE;
//}

/**
 * Add a command to the toolpath
 */
static PyObject *
PyTPGSettings_addSettingDefinition (cam_PyTPGSettings* self, PyObject* args) {

    //Expects (<action>, <setting>)

	const char *action;
	PyObject *settingObj;

	// extract arguments
    if (!PyArg_ParseTuple(args, "sO", &action, &settingObj)) {
        PyErr_SetString(PyExc_TypeError, "addSettingDefinition(..): expects 2 args, '<action>, <setting>' (str, object)");
        return NULL;
    }

    // check object type
    if (!PySettingsDefinition_Check(settingObj)) {
        PyErr_SetString(PyExc_TypeError, "<setting> parameter must be a Cam.TPGSettingDefinition instance");
        return NULL;
    }

    // add the setting to the underlying C++ settings structure
    cam_PySettingsDefinition *settingPy = (cam_PySettingsDefinition*) settingObj;
    QString qaction = QString::fromAscii(action);
    self->settings->addSettingDefinition(qaction, settingPy->setting);

    //TODO: return the setting object here so user can add options.  Need to define a new PySettingsDefinition type
//    Py_INCREF(settingPy);
//    return settingPy;
    Py_RETURN_NONE;
}

/**
 * Get selected action
 */
static PyObject *
PyTPGSettings_getAction (cam_PyTPGSettings* self, PyObject* args) {

	if (self->settings != NULL) {
		QString qaction = self->settings->getAction();
		return PyString_FromString(qaction.toAscii().constData());
	}

	PyErr_SetString(PyExc_Exception, "Underlying data-structure not set.");
	return NULL;
}

/**
 * Set selected action
 */
static PyObject *
PyTPGSettings_setAction (cam_PyTPGSettings* self, PyObject* args) {

	const char *action = NULL;

    if (!PyArg_ParseTuple(args, "s", &action)) {
		PyErr_SetString(PyExc_TypeError, "Expects '<action>' (1 string)");
		return NULL;
	}

    if (self->settings != NULL) {
    	QString qaction = QString::fromAscii(action);
    	bool result = self->settings->setAction(qaction);
		return PyBool_FromLong(result?1:0);
	}

	PyErr_SetString(PyExc_Exception, "Underlying data-structure not set.");
	return NULL;
}

/**
 * Get the value of a setting
 */
static PyObject *
PyTPGSettings_getValue (cam_PyTPGSettings* self, PyObject* args) {

	const char *action = NULL;
    const char *name = NULL;

    if (!PyArg_ParseTuple(args, "s|s", &action, &name)) {
		PyErr_SetString(PyExc_TypeError, "Expects '[<action>, ]<name>' (1 or 2 strings)");
		return NULL;
	}


    // get the value
    if (self->settings != NULL) {
        QString value;
		if (name == NULL) {
			QString qname = QString::fromAscii(action);
			value = self->settings->getValue(qname);
		} else {
			QString qname = QString::fromAscii(name);
			QString qaction = QString::fromAscii(action);
			value = self->settings->getValue(qaction, qname);
		}

	    // pack the value for return to python
	    return PyString_FromString(value.toAscii().constData());
    }

	PyErr_SetString(PyExc_Exception, "Underlying data-structure not set.");
	return NULL;
}

/**
 * Set the value of a setting
 */
static PyObject *
PyTPGSettings_setValue (cam_PyTPGSettings* self, PyObject* args) {

	const char *var1 = NULL;
    const char *var2 = NULL;
    const char *var3 = NULL;

    if (!PyArg_ParseTuple(args, "ss|s", &var1, &var2, &var3)) {
		PyErr_SetString(PyExc_TypeError, "Expects '[<action>, ]<name>, <value>' (2 or 3 strings)");
		return NULL;
	}

    // update the settings
    if (self->settings != NULL) {
    	bool result;
		if (var3 == NULL) {
			QString qname = QString::fromAscii(var1);
			QString qvalue = QString::fromAscii(var2);
			result = self->settings->setValue(qname, qvalue);
		} else {
			QString qaction = QString::fromAscii(var1);
			QString qname = QString::fromAscii(var2);
			QString qvalue = QString::fromAscii(var3);
			result = self->settings->setValue(qaction, qname, qvalue);
		}
		return PyBool_FromLong(result?1:0);
    }

	PyErr_SetString(PyExc_Exception, "Underlying data-structure not set.");
	return NULL;
}

/**
 * Adds any missing setting values (with their default value)
 */
static PyObject *
PyTPGSettings_addDefaults (cam_PyTPGSettings* self, PyObject* args) {

	if (self->settings != NULL)
		self->settings->addDefaults();

	PyErr_SetString(PyExc_Exception, "Underlying data-structure not set.");
	return NULL;
}

/**
 * Gets the action names defined in this settings object
 */
static PyObject *PyTPGSettings_getActions (cam_PyTPGSettings* self, PyObject* args) {

	if (self->settings != NULL) {
		// get the actions
		QStringList sl = self->settings->getActions();

		// convert to python list
		PyObject *actionList = PyList_New(sl.size());
		if (actionList != NULL) {
			for (int i = 0; i < sl.size(); i++)
				PyList_SET_ITEM(actionList, i, PyString_FromString(sl.at(i).toAscii().constData()));
			return actionList;
		}

		PyErr_SetString(PyExc_Exception, "Unable to create new list.");
		return NULL;
	}

	PyErr_SetString(PyExc_Exception, "Underlying data-structure not set.");
	return NULL;
}


///**
// * Clones the settings collection
// */
//static PyObject *
//PyTPGSettings_clone (cam_PyTPGSettings* self, PyObject* args) {
//
//	if (self->settings != NULL) {
//		Cam::Settings::TPGSettings* sett = self->settings->clone();
//		PyObject result = PyObject_New(, &PyTPGSettingsType);
//	}
//
//	PyErr_SetString(PyExc_Exception, "Underlying data-structure not set.");
//	return NULL;
//}

/**
 * Wrapper function to create new PyTPGSettings objects from c++
 */
extern PyObject* PyTPGSettings_New(Cam::Settings::TPGSettings* settings) {
    cam_PyTPGSettings *self;

    self = (cam_PyTPGSettings *)PyTPGSettingsType.tp_alloc(&PyTPGSettingsType, 0);
    if (self != NULL) {
        self->settings = settings->grab();
    }

    return (PyObject *)self;
}



namespace Cam
{
	namespace Settings
	{
		PyOption::PyOption()
		{
			this->option = new Option(QString::null, QString::null);
		}
		
		PyOption::~PyOption()
		{
			if (this->option != NULL)
			{
				delete (this->option);
				this->option = NULL;
			}
		}
			
		PyOption::PyOption(Option *option)
		{
			this->option = option;
		}

		PyOption::PyOption(Py::Object & object)
		{
			if (object.ptr()->ob_type == this->type_object()->ob_type)
			{
				printf("The types match\n");
			}
			else
			{
				printf("The types do NOT match\n");
			}
		}

		/* static */ void PyOption::init_type()
		{
			behaviors().name("Option");

			behaviors().doc("Cam::Settings::Option class");
			behaviors().supportStr();
			behaviors().supportGetattro();
			behaviors().supportSetattro();

			behaviors().readyType();
		}

		/* virtual */ Py::Object PyOption::str()
		{
			if (this->option != NULL)
			{
				QString xml;
				xml << *(this->option);
				return(Py::String(xml.toStdString()));
			}
			else
			{
				return PythonExtension::str();
			}
		}

		/* virtual */ Py::Object PyOption::getattro( const Py::String &name )
		{
			if (this->option == NULL)
			{
				return Py::None();
			}

			if (name == Py::String("id"))
			{
				return(Py::String(option->id.toStdString()));
			}
			else if (name == Py::String("label"))
			{
				return(Py::String(option->id.toStdString()));
			}
			else
			{
				// It might be a method name.  If so return a pointer to that method.
				return getattr_methods(name.as_std_string().c_str());
			}
		}

		/* virtual */ int	PyOption::setattro( const Py::String &name, const Py::Object &value )
		{
			if (this->option == NULL)
			{
				return(-1);
			}
		
			if (name == Py::String("id"))
			{
				this->option->id = QString::fromStdString(Py::String(value).as_std_string("utf-8"));
				return(0);
			}
			else if (name == Py::String("label"))
			{
				this->option->label = QString::fromStdString(Py::String(value).as_std_string("utf-8"));
				return(0);
			}
			else
			{
				return genericSetAttro(name, value);
			}
		}


		Py::String PyOption::id() const
		{
			if (this->option == NULL)
			{
				throw(Py::Exception("No option pointer found"));
			}
			else
			{
				return(Py::String(this->option->id.toStdString()));
			}
		}

		Py::String PyOption::label() const
		{
			if (this->option == NULL)
			{
				throw(Py::Exception("No option pointer found"));
			}
			else
			{
				return(Py::String(this->option->label.toStdString()));
			}
		}







		PyDefinition::PyDefinition()
		{
			this->definition = new Definition();
		}
		
		PyDefinition::~PyDefinition()
		{
			if (this->definition != NULL)
			{
				this->definition->release();
				this->definition = NULL;
			}
		}
			
		PyDefinition::PyDefinition(Definition *definition)
		{
			this->definition = definition->grab();
		}

		/* static */ void PyDefinition::init_type()
		{
			behaviors().name("Definition");

			behaviors().doc("Cam::Settings::Definition class");
			behaviors().supportStr();
			behaviors().supportGetattro();	// If we support the getattro() and setattro() methods then they are used INSTEAD of the getattr()/setattr() methods (i.e. the ones without the 'o' on the end of the name)
			behaviors().supportSetattro();			

			// NOTE: We MUST add the getattr_methods() call within either the getattr() or getattro() methods
			// in order to be able to call these exported methods.  It's not enough to just add the following
			// lines to this init_type() method.
			add_keyword_method( "addOption", &PyDefinition::addOption, "addOption(id, label) id and label are both strings");
	
			behaviors().readyType();
		}

		/* virtual */ Py::Object PyDefinition::str()
		{
			if (this->definition != NULL)
			{
				QString xml;
				xml << *definition;

				return(Py::String(xml.toStdString()));
			}
			else
			{
				return PythonExtension::str();
			}
		}

		/**
			We MUST implement the getattro() or getattr() methods if we want to
			expose a method within the class.  It's the getattr_methods() call
			at the bottom of this method that does the work necessary to return
			a pointer to the exported method. (and call it).

			This method is called whenever we use the ".something" sequence following
			an object name in Python.  eg:
			import Settings
			myobject = Settings.Definition()
			myobject.name = 'my name'
			myobject.addOption(id='my id', label='my label')

			In the above example, this getattro() method is called TWICE.  Once with
			a name of 'name' (from the myobject.name reference) and the other time
			with a name of 'addOption'.  The 'addOption' name is handled by the
			getattr_methods() call at the bottom of this method.
		 */
		/* virtual */ Py::Object PyDefinition::getattro( const Py::String &name )
		{
			if (this->definition == NULL)
			{
				return Py::None();
			}

			if (name == Py::String("name"))
			{
				return(Py::String(definition->name.toStdString()));
			}
			else if (name == Py::String("label"))
			{
				return(Py::String(definition->label.toStdString()));
			}
			else if (name == Py::String("defaultvalue"))
			{
				return(Py::String(definition->defaultvalue.toStdString()));
			}
			else if (name == Py::String("units"))
			{
				return(Py::String(definition->units.toStdString()));
			}
			else if (name == Py::String("helptext"))
			{
				return(Py::String(definition->helptext.toStdString()));
			}
			else if (name == Py::String("visible"))
			{
				return(Py::Boolean(definition->visible));
			}
			else if (name == Py::String("type"))
			{
				QString verbose;
				verbose << definition->type;
				return(Py::String(verbose.toStdString()));
			}
			else if (name == Py::String("options"))
			{
				Py::List options;
				for (QList<Option*>::const_iterator itOption = definition->options.begin(); itOption != definition->options.end(); itOption++)
				{
					options.append(Py::asObject(new PyOption(*itOption)));
				}

				return(options);
			}
			else
			{
				// It might be a method name.  If so return a pointer to that method.
				return getattr_methods(name.as_std_string().c_str());
			}
		}

		/* virtual */ int	PyDefinition::setattro( const Py::String &name, const Py::Object &value )
		{
			if (this->definition == NULL)
			{
				return(-1);
			}
		
			if (name == Py::String("name"))
			{
				definition->name = QString::fromStdString(Py::String(value).as_std_string("utf-8"));
				return(0);
			}
			else if (name == Py::String("label"))
			{
				definition->label = QString::fromStdString(Py::String(value).as_std_string("utf-8"));
				return(0);
			}
			else if (name == Py::String("defaultvalue"))
			{
				definition->defaultvalue = QString::fromStdString(Py::String(value).as_std_string("utf-8"));
				return(0);
			}
			else if (name == Py::String("units"))
			{
				definition->units = QString::fromStdString(Py::String(value).as_std_string("utf-8"));
				return(0);
			}
			else if (name == Py::String("helptext"))
			{
				definition->helptext = QString::fromStdString(Py::String(value).as_std_string("utf-8"));
				return(0);
			}
			else if (name == Py::String("visible"))
			{
				definition->visible = Py::Boolean(value);
				return(0);
			}
			else if (name == Py::String("type"))
			{
				std::ostringstream error;
				error << "Unexpected type '" << value.as_string() << "'.  Value must be one of ";

				for (Definition::SettingType setting_type = Definition::SettingType_Text; setting_type <= Definition::SettingType_Rate; setting_type = Definition::SettingType(int(setting_type)+1))
				{
					QString verbose;
					verbose << setting_type;	// Convert between the enum and the string version.
					if (setting_type != Definition::SettingType_Text)
					{
						error << ", ";
					}
					error << verbose.toStdString();

					if (value == Py::String(verbose.toStdString()))
					{
						definition->type = setting_type;
						return(0);
					}
				}

				throw(Py::Exception(error.str()));
			}
			else if (name == Py::String("options"))
			{
				for (QList<Option*>::const_iterator itOption = definition->options.begin(); itOption != definition->options.end(); itOption++)
				{
					delete *itOption;
				}

				definition->options.clear();
			
				Py::List new_options(value);
				for (Py::List::size_type i=0; i<new_options.size(); i++)
				{
					printf("Point 1.1\n");
					PyOption option;
					// Py::Type type1(option.self());
					/*
					if (new_options[i].isType(Py::Type(option.selfPtr())))
					{
						printf("yes\n");
					}
					else
					{
						printf("no\n");
					}
					*/

					/*
					PyOption *pPyOption = static_cast<PyOption *>(new_options[i].ptr());
					printf("Point two\n");

					if (pPyOption)
					{
						printf("obj is just fine\n");

						printf("id is %s\n", pPyOption->id().as_std_string().c_str());
						definition->addOption(QString::fromStdString(pPyOption->id().as_std_string()), QString::fromStdString(pPyOption->label().as_std_string()));
						printf("Point four\n");
					}
					else
					{
						printf("obj is empty\n");
					}
					*/
				}

				return(0);
			}
			else
			{
				return genericSetAttro(name, value);
			}
		}


		Py::Object PyDefinition::addOption( const Py::Tuple &args, const Py::Dict &kwds )
		{
			if (kwds.hasKey("id") && kwds.hasKey("label"))
			{
				Py::List keys = kwds.keys();
				Py::List values = kwds.values();

				QString initial_value = QString::fromAscii("Unassigned");
				QString id = initial_value;
				QString label = initial_value;

				for (Py::List::size_type i=0; i<keys.size(); i++)
				{
					if (keys[i] == Py::String("id")) id = QString::fromStdString( values[i].str().as_string().c_str() );
					if (keys[i] == Py::String("label")) label = QString::fromStdString( values[i].str().as_string().c_str() );
				}

				if ((id == initial_value) || (label == initial_value))
				{
					throw(Py::Exception("addOption() method MUST be provided with both 'id' and 'label' named arguments"));
				}
				else
				{
					this->definition->addOption( id, label );
					return(Py::None());
				}
			}
			else
			{
				throw(Py::Exception("addOption() method MUST be provided with both 'id' and 'label' named arguments"));
			}
		}		





		PyTPGSettings::PyTPGSettings()
		{
			this->settings = new TPGSettings();
		}
		
		PyTPGSettings::~PyTPGSettings()
		{
			if (this->settings != NULL)
			{
				this->settings->release();
				this->settings = NULL;
			}
		}
			
		PyTPGSettings::PyTPGSettings(TPGSettings *settings)
		{
			this->settings = settings->grab();
		}

		/* static */ void PyTPGSettings::init_type()
		{
			behaviors().name("TPGSettings");

			behaviors().doc("Cam::Settings::TPGSettings class");
			behaviors().supportStr();
			behaviors().supportGetattro();	// If we support the getattro() and setattro() methods then they are used INSTEAD of the getattr()/setattr() methods (i.e. the ones without the 'o' on the end of the name)
			behaviors().supportSetattro();
	
			behaviors().readyType();
		}

		/* virtual */ Py::Object PyTPGSettings::str()
		{
			if (this->settings != NULL)
			{
				QString xml;
				xml << *settings;

				return(Py::String(xml.toStdString()));
			}
			else
			{
				return PythonExtension::str();
			}
		}

		/* virtual */ Py::Object PyTPGSettings::getattro( const Py::String &name )
		{
			if (this->settings == NULL)
			{
				return Py::None();
			}

			// It might be a method name.  If so return a pointer to that method.
			return getattr_methods(name.as_std_string().c_str());
		}

		/* virtual */ int	PyTPGSettings::setattro( const Py::String &name, const Py::Object &value )
		{
			if (this->settings == NULL)
			{
				return(-1);
			}
		
			return genericSetAttro(name, value);
		}






static PySettingsModule *pySettings_static_reference;

extern "C" CamExport void init()
{
#if defined(PY_WIN32_DELAYLOAD_PYTHON_DLL)
    Py::InitialisePythonIndirectPy::Interface();
#endif

    pySettings_static_reference = new PySettingsModule;
}

// symbol required for the debug version
extern "C" CamExport void init_d()
{ 
    init();
}

		
	}; // End namespace Settings
}; // End namespace Cam

