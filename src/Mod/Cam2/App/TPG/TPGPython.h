/***************************************************************************
 *   Copyright (c) 2012 Luke Parry    (l.parry@warwick.ac.uk)              *
 *                 2012 Andrew Robinson  (andrewjrobinson@gmail.com)       *
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

#ifndef CAM_TPGPYTHON_H
#define CAM_TPGPYTHON_H

#include "TPG.h"
#include "ToolPath.h"

extern void Message(const char*);

void PythonInit();
void PythonFinish();

namespace Cam
{

// Class stores hash of settings for managing each independant TPG
class CamExport TPGPython : public TPG
{
public:
    TPGPython(PyObject *cls);
    ~TPGPython();

    // TPG API methods.  Used by the CAM workbench to run the TPG

    /**
     * Returns a list of action names supported by this TPG.
     *
     * Note: if this TPG only does one thing then just return a list containing 'default'
     */
    virtual std::vector<QString> &getActions();

    /**
     * Get the settings for the TPG
     */
	virtual Settings::TPGSettings *getSettingDefinitions();

    /**
     * Run the TPG to generate the ToolPath code.
     *
     * Note: the return will change once the TP Language has been set in store
     */
    virtual void run(Settings::TPGSettings *settings, ToolPath *toolpath, QString action);

//    /**
//     * Returns the toolpath from the last
//     */
//    virtual ToolPath *getToolPath();

    virtual QString getId();
    virtual QString getName();
    virtual QString getDescription();

protected:
    PyObject *cls; ///< the python class that this object is wrapping
    PyObject *inst; ///< the instance of cls.  Only created on the first call to a method other that getId, getName, or getDescription.

    QString PythonUCToQString(PyObject *obj);
    PyObject *QStringToPythonUC(const QString &str);

	PyObject *QStringListToPythonUCList(const QStringList &string_list);
	QStringList PythonUCListToQStringList(PyObject *obj);


    /**
     * Creates an instance of cls and stores it in inst if it doesn't exist
     */
    PyObject *getInst(void);
};

} //namespace Cam

#endif //CAM_TPGPYTHON_H
