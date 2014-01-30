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

#ifndef MACHINEPROGRAM_H_
#define MACHINEPROGRAM_H_

namespace Cam {
class CamExport MachineProgram;
}

#include <QStringList>
#include "TPG/ToolPath.h"

namespace Cam {

/**
	The MachineProgram class contains the output from executing the
	Python scripts housed within a single ToolPath object.  It holds
	a QStringList for the stdout (GCode) and another for the errors
	(including execptions).

	An instance of this class is constructed by the PostProcessor::postProcess()
	method.  The MachineProgram pointer returned must be released
	by the calling routine.  i.e. call the release() method and allow
	that to free any memory when the refcnt reaches zero.
 */
class CamExport MachineProgram {
protected:
    QStringList *machineProgram;
	QStringList *errors;
	ToolPath	*toolPath;

	/**
		We want to keep track of which lines in the ToolPath object (Python code)
		result in which lines in this MachineProgram (GCode) object.  In the end
		it would be nice to be able to click on a piece of GCode and see which
		lines of Python produced them.  This is just part of that linkage.
	 */
	typedef QStringList::size_type MachineProgramOffset_t;
	typedef QStringList::size_type ToolPathOffset_t;
	typedef std::multimap< ToolPathOffset_t, MachineProgramOffset_t > Indices_t;

	Indices_t	indices;

    int refcnt;
    virtual ~MachineProgram();
public:
    MachineProgram(ToolPath *toolPath);
    MachineProgram(const std::vector<std::basic_string<char> >&, ToolPath *toolPath);

    /**
     * Add a single Machine command to the Program
	 * The toolpath_offset is the zero-based offset of the QStringList
	 * in the ToolPath::toolpath member.
     */
	void addMachineCommand(QString mc, QStringList::size_type toolpath_offset);

	void addErrorString(QString error_string);

    /**
     * Clear out the toolpath.
     */
    void clear();

    /**
     * Get the Machine Program as a list of strings.  This comes from the
	 * stdout of the Python interpreter when executing the python code
	 * held within the ToolPath object.
     */
    QStringList *getMachineProgram();

	/**
	 * Return any error strings (including exceptions) that were seen during the
	 * execution of the ToolPath (i.e. the python code).  It's only by looking
	 * at this QStringList that the calling routine can determine if any errors
	 * (including exceptions) occured during the execution of the ToolPath (i.e.
	 * the Python program).
	 */
	QStringList *getErrors();

	/**
	 * We keep a pointer to the ToolPath object used to create this MachineProgram
	 * object so that we can make sense of the index that associates which lines
	 * of the MachineProgram were produced by which lines of the ToolPath.
	 * NOTE: This method calls grab() to increment the reference counter.  The calling
	 * object MUST call release() when it's finished with it.
	 */
	ToolPath *getToolPath();

    /**
     * Increases reference count
     * Note: it returns a pointer to this for convenience.
     */
    MachineProgram *grab() {
        refcnt++;
        return this;
    }

    /**
     * Decreases reference count and deletes self if no other references
     */
    void release() {
        refcnt--;
        if (refcnt == 0)
            delete this;
    }

	/**
		Follow the indices that match the Python code found in the ToolPath object with the GCode found
		in this MachineProgram object.  Log these linkages to a QString for debugging purposes only.
	*/
	QString TraceProgramLinkages() const;

public:
	friend QString CamExport operator<< ( QString & buf, const MachineProgram & machine_program );
};

} /* namespace Cam */
#endif /* MACHINEPROGRAM_H_ */
