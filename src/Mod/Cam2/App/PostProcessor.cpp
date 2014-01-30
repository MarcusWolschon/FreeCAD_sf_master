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

#include "PostProcessor.h"
#include <Base/Interpreter.h>

#include <Base/PyTools.h>
#include <Base/Exception.h>
#include <Base/PyObjectBase.h>



namespace Cam {

PostProcessorInst* PostProcessorInst::_pcSingleton = NULL;

PostProcessorInst& PostProcessorInst::instance(void)
{
    if (_pcSingleton == NULL) {
        _pcSingleton = new PostProcessorInst;
    }

    return *_pcSingleton;
}

PostProcessorInst::PostProcessorInst() {
	// Register the callback functions for stdout and stderr in the
	// global Python interpreter.  We must do this only once or
	// an execption is thrown.  That's why we do it within the
	// constructor of this singleton class.

	PythonStdout::init_type();
	PythonStderr::init_type();
}

PostProcessorInst::~PostProcessorInst() {
}


/**
	Use the global Python interpreter to execute the lines of Python code
	in the ToolPath and capture both stdout and stderr into the MachineProgram
	object returned.  The stdout is expected to be the actual GCode.  The
	stderr, if any is seen, will indicate whether errors and/or execeptions
	were seen.

	NOTE: We MUST use the Global Interpreter Lock (Base::PyGILStateLocker) to
	ensure that toolpaths that are calculated in parallel, are not also executed
	in parallel here.  We MUST capture the stdout and stderr streams JUST for this
	instance of execution so that the output from this toolpath doesn't interfere
	with that from another toolpath.
 */
MachineProgram *PostProcessorInst::postProcess(ToolPath *toolpath, Item *postprocessor)
{
	// Define a new MachineProgram object to contain the GCode (stdout from the Python program)
	MachineProgram *machine_program = new MachineProgram(toolpath);

	// Lock the 'Global Interpreter Lock' so that we're not interrupted during our execution.
	Base::PyGILStateLocker locker;	

	// Remember where stdout and stderr were pointing before we start so that we can reinstate them later.
	PyObject *original_stdout = PySys_GetObject("stdout");
	PyObject *original_stderr = PySys_GetObject("stderr");

	// Redirect stdout and stderr from the Python interpreter so that it ends up
	// within the MachineProgram object.
	PythonStdout* out = new PythonStdout(machine_program);
    PySys_SetObject("stdout", out);

	PythonStderr *err = new PythonStderr(machine_program);
    PySys_SetObject("stderr", err);

	// For debug only.  Should remove this eventually.
	QString tool_path;
	tool_path << *toolpath;
	qDebug("%s\n", tool_path.toAscii().constData());

	// Execute the Python code line-by-line...
	QStringList::size_type i=0;
	try
	{
		QStringList *lines = toolpath->getToolPath();
		for (i=0; i<lines->size(); i++)
		{
			// Seed the PythonStdout object with the line offset from the toolpath QStringList
			// so that we can tie the generated GCode with the lines in the toolpath (Python) script.
			out->ToolPathIndex( QStringList::size_type(i) );
			err->ToolPathIndex( QStringList::size_type(i) );
			
			Base::Interpreter().runString(lines->at(i).toAscii().constData());			
		}
	}
	catch(Base::PyException & error)
	{
		qCritical("Error found at line %d\n%s\n%s\n", i+1, toolpath->getToolPath()->at(i).toAscii().constData(), error.what());	// send the exception message to the build environment's output
		machine_program->addErrorString(QString::fromAscii(error.what()));	// as well as to the machine program to indicate a failure has occured.
	}

	if (original_stdout != NULL) PySys_SetObject("stdout", original_stdout);
	if (original_stderr != NULL) PySys_SetObject("stderr", original_stderr);

	// Always return the machine_program which includes both gcode and any errors that were seen.  It's up
	// to the calling routine to look to see if any errors occured by looking at this machine_program object.
	return(machine_program);
}

/**
	This method gets called when a line of Python code attempts to write to stdout.
	We capture the string they were going to write and re-direct it to the
	machine_program->machineProgram QStringList.
 */
Py::Object PythonStdout::write(const Py::Tuple& args)
{
    try {
        Py::Object output(args[0]);
        if (PyUnicode_Check(output.ptr())) {
            PyObject* unicode = PyUnicode_AsEncodedObject(output.ptr(), "utf-8", "strict");
            if (unicode) {
                const char* string = PyString_AsString(unicode);
				buffer += QString::fromUtf8(string);
				if (buffer.endsWith(QString::fromAscii("\n")))
				{
					this->machine_program->addMachineCommand(buffer, this->toolpath_index);
					buffer.clear();
				}
                Py_DECREF(unicode);
            }
        }
        else {
            Py::String text(args[0]);
            std::string string = (std::string)text;
			buffer += QString::fromStdString(string);

			// Only add it to the machine_program list if it ends with a newline character.  Otherwise
			// we're implicitly adding a newline.  Until that time, just accumulate the characters for
			// a single line in buffer.
			if (buffer.endsWith(QString::fromAscii("\n")))
			{
				this->machine_program->addMachineCommand(buffer, this->toolpath_index);
				buffer.clear();
			}
        }
    }
    catch (Py::Exception& e) {
        // Do not provoke error messages 
        e.clear();
    }

    return Py::None();
}

Py::Object PythonStdout::flush(const Py::Tuple&)
{
    return Py::None();
}

Py::Object PythonStdout::repr()
{
    std::string s;
    std::ostringstream s_out;
    s_out << "PythonStdout";
    return Py::String(s_out.str());
}


/**
	WARNING: This MUST be done ONLY ONCE.  That's why we call it from
	the PostProcessorInst object's constructor.  i.e. it's a singleton.
 */
void PythonStdout::init_type()
{
    behaviors().name("PythonStdout");
    behaviors().doc("Redirection of stdout to the MachineProgram object.");
    // you must have overwritten the virtual functions
    behaviors().supportRepr();
    add_varargs_method("write",&PythonStdout::write,"write()");
    add_varargs_method("flush",&PythonStdout::flush,"flush()");
}

/**
	This method gets called when a line of Python code attempts to write to stderr.
	We capture the string they were going to write and re-direct it to the
	machine_program->error_string QStringList.
 */
Py::Object PythonStderr::write(const Py::Tuple& args)
{
    try {
        Py::Object output(args[0]);
        if (PyUnicode_Check(output.ptr())) {
            PyObject* unicode = PyUnicode_AsEncodedObject(output.ptr(), "utf-8", "strict");
            if (unicode) {
                const char* string = PyString_AsString(unicode);
				if (buffer.endsWith(QString::fromAscii("\n")))
				{
					this->machine_program->addErrorString(buffer);
					buffer.clear();
				}
                Py_DECREF(unicode);
            }
        }
        else {
            Py::String text(args[0]);
            std::string string = (std::string)text;
			buffer += QString::fromStdString(string);
            if (buffer.endsWith(QString::fromAscii("\n")))
			{
				this->machine_program->addErrorString(buffer);
				buffer.clear();
			}
        }
    }
    catch (Py::Exception& e) {
        // Do not provoke error messages 
        e.clear();
    }

    return Py::None();
}

Py::Object PythonStderr::flush(const Py::Tuple&)
{
    return Py::None();
}

Py::Object PythonStderr::repr()
{
    std::string s;
    std::ostringstream s_out;
    s_out << "PythonStderr";
    return Py::String(s_out.str());
}

/**
	WARNING: This MUST be done ONLY ONCE.  That's why we call it from
	the PostProcessorInst object's constructor.  i.e. it's a singleton.
 */
void PythonStderr::init_type()
{
    behaviors().name("PythonStderr");
    behaviors().doc("Redirection of stdout to the MachineProgram object.");
    // you must have overwritten the virtual functions
    behaviors().supportRepr();
    add_varargs_method("write",&PythonStderr::write,"write()");
    add_varargs_method("flush",&PythonStderr::flush,"flush()");
}

} /* namespace Cam */
