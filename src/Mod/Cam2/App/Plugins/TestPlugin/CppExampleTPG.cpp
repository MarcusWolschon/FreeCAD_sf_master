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

#include "PreCompiled.h"
#ifndef _PreComp_
#endif

#include <TPG/CppTPGDescriptor.h>

#include "CppExampleTPG.h"
#include "Graphics/Paths.h"

#define myID   "95744f1e-360f-11e2-bcd3-08002734b94f"
#define myName "Example CPP TPG"
#define myDesc "A simple example CPP TPG to demonstrating how to create one. "

/**
 * Implement the Cpp shared library interface functions
 * Note: this must be outside the namespace declaration
 */
//CPPTPG_API_SOURCE(Cam::CppExampleTPG, myID, myName, myDesc)

//TODO: put this back once the memory corruption in getTPG() is located.
extern "C" Q_DECL_EXPORT Cam::TPGDescriptorCollection* getDescriptors() {
    Cam::TPGDescriptorCollection* descriptors = new Cam::TPGDescriptorCollection();
    Cam::TPGDescriptor *descriptor = new Cam::CppTPGDescriptor(QString::fromAscii("95744f1e-360f-11e2-bcd3-08002734b94f"), QString::fromAscii("Example CPP TPG"), QString::fromAscii("A simple example CPP TPG to demonstrating how to create one. "));
    descriptors->add(descriptor);
    descriptor->release();
    return descriptors;
}
extern "C" Q_DECL_EXPORT Cam::CppTPG* getTPG(QString id) {
    if (id.compare(QString::fromAscii(myID)) == 0)
        return new Cam::CppExampleTPG();
    return 0;
}

/// TPG Implementation ///
namespace Cam {

CppExampleTPG::CppExampleTPG()
        : CppTPG() { // important to call parent constructor
    id = QS(myID);
    name = QS(myName);
    description = QS(myDesc);

    QString qaction = QS("default");
    actions.push_back(qaction);
    settings = new TPGSettings();
    settings->addSettingDefinition(qaction, new TPGSettingDefinition("geometry", "Geometry", "Cam::TextBox", "Box01", "", "The input geometry that should be cut"));
    settings->addSettingDefinition(qaction, new TPGSettingDefinition("tool", "Tool", "Cam::TextBox", "Tool01", "", "The tool to use for cutting"));
    TPGSettingDefinition* speed = settings->addSettingDefinition(qaction, new TPGSettingDefinition("speed", "Speed", "Cam::Radio", "normal", "", "The speed of the algorithm.  Faster will use less accurate algorithm."));
    speed->addOption("fast", "Fast");
    speed->addOption("normal", "Normal");
    speed->addOption("slow", "Slow");
}

CppExampleTPG::~CppExampleTPG() {
    
}

/**
 * Run the TPG to generate the ToolPath code.
 *
 * Note: the return will change once the TP Language has been set in stone
 */
void CppExampleTPG::run(TPGSettings *settings, QString action= QString::fromAscii(""))
{
    qDebug("This is where the TPG would generate the tool-path! \n");

	/*
	// just for testing the getToolPath() method.
	ToolPath *pToolPath = getToolPath();
	QString tool_path;
	tool_path << *pToolPath;
	qDebug(tool_path.toAscii().constData());
	pToolPath->release();
	*/
}


/* virtual */ ToolPath *CppExampleTPG::getToolPath()
{
	ToolPath *pPython = new ToolPath(this);

	pPython->RequiredDecimalPlaces(3);	// assume metric.
	*pPython << "rapid(x=" << 12.3456789 << ")\n";
	*pPython << "feed(x=" << 4 << ")\n";

	QString some_setting(QString::fromUtf8("'Something (that) needs cleaning up befor it's included in a python comment() call'"));
	*pPython << "comment(" << pPython->PythonString(some_setting) << ")\n";

	*pPython << "comment(" << pPython->PythonString("the cat's mat has a single quote in it") << ")\n";
	*pPython << "comment(" << pPython->PythonString("this one has (something in brackets) within it.") << ")\n";
	*pPython << "comment(" << pPython->PythonString("'this one already has single quotes bounding it'") << ")\n";
	*pPython << "comment(" << pPython->PythonString("\"this one already has double quotes bounding it\"") << ")\n";

	return(pPython);
}


} /* namespace Cam */
