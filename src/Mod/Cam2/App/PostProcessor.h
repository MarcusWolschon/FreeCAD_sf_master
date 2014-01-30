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

#ifndef POSTPROCESSOR_H_
#define POSTPROCESSOR_H_

#include <cstdlib>

#include "TPG/ToolPath.h"
#include "ItemList.h"
#include "MachineProgram.h"

#include <CXX/Extensions.hxx>


namespace Cam {

	/**
	 * Python class for redirection of stdout to the MachineProgram
	 * object.  This was copied from the App/Gui/PythonConsolePy.h file
	 * @see PythonStderr
	 * @see PythonConsole
	 * @author Werner Mayer
	 */
	class PythonStdout : public Py::PythonExtension<PythonStdout> 
	{
	private:
		MachineProgram* machine_program;
		QString buffer;
		QStringList::size_type toolpath_index;	// Which line of the toolpath QStringList did this machine_program code come from?

	public:
		PythonStdout(MachineProgram *machine_program)
		{
			this->machine_program = machine_program->grab();
		}
		~PythonStdout()
		{
			this->machine_program->release();
		}

		void ToolPathIndex(QStringList::size_type index)
		{
			this->toolpath_index = index;
		}

		static void init_type(void);    // announce properties and methods

		Py::Object repr();
		Py::Object write(const Py::Tuple&);
		Py::Object flush(const Py::Tuple&);
	};

	/**
	 * Python class for redirection of stdout to the MachineProgram
	 * object.  This was copied from the App/Gui/PythonConolePy.h file
	 * @see PythonStderr
	 * @see PythonConsole
	 * @author Werner Mayer
	 */
	class PythonStderr : public Py::PythonExtension<PythonStderr> 
	{
	private:
		MachineProgram* machine_program;
		QString buffer;
		QStringList::size_type toolpath_index;	// Which line of the toolpath QStringList did this machine_program code come from?

	public:
		PythonStderr(MachineProgram *machine_program)
		{
			this->machine_program = machine_program->grab();
		}
		~PythonStderr()
		{
			this->machine_program->release();
		}

		void ToolPathIndex(QStringList::size_type index)
		{
			this->toolpath_index = index;
		}

		static void init_type(void);    // announce properties and methods

		Py::Object repr();
		Py::Object write(const Py::Tuple&);
		Py::Object flush(const Py::Tuple&);
	};

	class PostProcessorInst {

protected:
    static PostProcessorInst* _pcSingleton;

    PostProcessorInst();
    virtual ~PostProcessorInst();

public:
    static PostProcessorInst& instance(void);
    static void destruct (void);


    Cam::ItemList *getPostProcessorList(bool rescan);
    Cam::MachineProgram *postProcess(Cam::ToolPath *toolpath, Cam::Item *postprocessor);
};

/// Get the global instance
inline PostProcessorInst& PostProcessor(void)
{
    return PostProcessorInst::instance();
}

} /* namespace Cam */
#endif /* POSTPROCESSOR_H_ */
