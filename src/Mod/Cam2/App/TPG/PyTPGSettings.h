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

#ifndef PYTPGSETTINGS_H_
#define PYTPGSETTINGS_H_

#include <Python.h>
#include <QString>

#include "TPGSettings.h"

#include "CXX/Objects.hxx"
#include "CXX/Extensions.hxx"


#define QString_toPyString(__str__) PyString_FromString(((const char*)(__str__)).toStdString().c_str());
#define QStringPtr_toPyString(__str__) PyString_FromString(((const char*)(__str__))->toStdString().c_str());

extern PyTypeObject *PyTPGSettings_Type();
extern PyTypeObject *PySettingsDefinition_Type();


#define PyCamTPGSettings_Check(__obj__) PyObject_TypeCheck((__obj__), PyTPGSettings_Type())
#define PySettingsDefinition_Check(__obj__) PyObject_TypeCheck((__obj__), PySettingsDefinition_Type())


// ----------------------------------------------------------------------------
// ----- TPGSettingDefinition -------------------------------------------------
// ----------------------------------------------------------------------------

/**
 * A wrapper for the TPGSettingDefinition class to allow access from PyTPGs
 */
typedef struct  {
    PyObject_HEAD
    Cam::Settings::Definition *setting;
} cam_PySettingsDefinition;



// ----------------------------------------------------------------------------
// ----- TPGSettings ----------------------------------------------------------
// ----------------------------------------------------------------------------

/**
 * A wrapper for the TPGSettings class to allow access from PyTPGs
 */
typedef struct  {
    PyObject_HEAD
    Cam::Settings::TPGSettings *settings;
} cam_PyTPGSettings;

/**
 * Wrapper function to create new PyTPGSettings objects from c++
 */
extern PyObject* PyTPGSettings_New(Cam::Settings::TPGSettings* settings);

namespace Cam
{
	namespace Settings
	{
		/**
			Python wrapper for the Cam::Settings::Option class.
		 */
		class CamExport PyOption : public Py::PythonExtension<PyOption>
		{
		public:
			PyOption();
			~PyOption();
			PyOption(Option *pOption);
			PyOption(Py::Object & object);

		public:
			// Py::PythonExtension framework methods.
			static void init_type();

			virtual Py::Object	str();
			virtual Py::Object	getattro( const Py::String &name );
			virtual int			setattro( const Py::String &name, const Py::Object &value );

		public:
			Py::String id() const;
			Py::String label() const;

		private:
			Option	*option;	// Pointer to a C++ object that contains the 'real' data.
		}; // End PyOption class definition


		class CamExport PyDefinition : public Py::PythonExtension<PyDefinition>
		{
		public:
			PyDefinition();
			~PyDefinition();
			PyDefinition(Definition *pDefinition);

		public:
			// Py::PythonExtension framework methods.
			static void init_type();

			virtual Py::Object	str();
			virtual Py::Object	getattro( const Py::String &name );
			virtual int			setattro( const Py::String &name, const Py::Object &value );

		public:
			Py::Object addOption( const Py::Tuple &args, const Py::Dict &kwds );

		private:
			Definition	*definition;	// Pointer to a C++ object that contains the 'real' data.
		}; // End PyDefinition class definition


		class CamExport PyTPGSettings : public Py::PythonExtension<PyTPGSettings>
		{
		public:
			PyTPGSettings();
			~PyTPGSettings();
			PyTPGSettings(TPGSettings *pSettings);

		public:
			// Py::PythonExtension framework methods.
			static void init_type();

			virtual Py::Object	str();
			virtual Py::Object	getattro( const Py::String &name );
			virtual int			setattro( const Py::String &name, const Py::Object &value );

		private:
			TPGSettings	*settings;	// Pointer to a C++ object that contains the 'real' data.
		}; // End PyTPGSettings class definition

		

		/**
			The PySettingsModule class exists as the 'Settings' module within the 'Cam' module.
			i.e. when in a Python script, one must "import Cam" and then one is able to
			"import Settings".  That "import Settings" command instantiates a copy of this
			PySettingsModule class.

			When the Python script then does a "myObject = Settings.Option()", it's calling
			the PySettingsModule::factory_PyOption() method in this class due to the
			add_varargs_method() call with the 'name' of 'Option'.  The returned object
			(myObject) is an instance of a Cam::Settings::PyOption class.  i.e. the
			Python script can then make calls such as 'myObject.id = "my id"' and
			'myObject.label = "my label"'.

			This PySettingsModule class will wrap up ALL classes within the Cam::Settings namespace
			of the C++ version.
		 */
		class PySettingsModule : public Py::ExtensionModule<PySettingsModule>
		{
		public:
			PySettingsModule() : Py::ExtensionModule<PySettingsModule>("Settings")
			{
				PyOption::init_type();
				PyDefinition::init_type();
		
				add_varargs_method("Option", &PySettingsModule::factory_PyOption, "Method to instantiate a new Cam::Settings::Option");
				add_varargs_method("Definition", &PySettingsModule::factory_PyDefinition, "Method to instantiate a new Cam::Settings::Definition");
				add_varargs_method("TPGSettings", &PySettingsModule::factory_PyTPGSettings, "Method to instantiate a new Cam::Settings::TPGSettings");

				initialize("The Settings module includes classes for Option, Definition and TPGSettings.");
			}

			virtual ~PySettingsModule() {}

			Py::Object factory_PyOption( const Py::Tuple &rargs )
			{
				Py::Object obj = Py::asObject( new PyOption );
				return(obj);
			}

			Py::Object factory_PyDefinition( const Py::Tuple &rargs )
			{
				Py::Object obj = Py::asObject( new PyDefinition );
				return(obj);
			}

			Py::Object factory_PyTPGSettings( const Py::Tuple &rargs )
			{
				Py::Object obj = Py::asObject( new PyTPGSettings );
				return(obj);
			}
		}; // End PySettingsModule class definition

		extern "C" CamExport void init();
	} // End namespace Settings
} // End namespace Cam


#endif /* PYTPGSETTINGS_H_ */
