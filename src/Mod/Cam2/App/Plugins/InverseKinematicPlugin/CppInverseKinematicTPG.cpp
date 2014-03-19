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

#include <Base/Interpreter.h>		// For Python runtime
#include <App/PropertyUnits.h>
#include <TPG/CppTPGDescriptor.h>
#include <Mod/Part/App/PrimitiveFeature.h>

#include <App/Document.h>
#include <App/DocumentObject.h>
#include <App/Application.h>

#include "CppInverseKinematicTPG.h"
#include "Graphics/Paths.h"
#include <Mod/Cam2/App/GCode.h>

#include <QRegExp>

//TODO: how to get an id? randomly?
#define myID   "12345678-9abc-def0-0000-000000000000"
#define myName "Inverse Kinematic CPP TPG"
#define myDesc "A TPG turning the output of a 3 axis TPG 4 axis."


#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_increment_actor.hpp>

using namespace BOOST_SPIRIT_CLASSIC_NS;


/**
 * Implement the Cpp shared library interface functions
 * Note: this must be outside the namespace declaration
 */
//CPPTPG_API_SOURCE(Cam::CppInverseKinematicTPG, myID, myName, myDesc)
//TODO: put this back once the memory corruption in getTPG() is located.
extern "C" Q_DECL_EXPORT Cam::TPGDescriptorCollection* getDescriptors() {
    Cam::TPGDescriptorCollection* descriptors = new Cam::TPGDescriptorCollection();
    Cam::TPGDescriptor *descriptor = new Cam::CppTPGDescriptor(QString::fromAscii(myID),
							       QString::fromAscii(myName),
							       QString::fromAscii(myDesc));
    descriptors->add(descriptor);
    descriptor->release();
    return descriptors;
}
extern "C" Q_DECL_EXPORT Cam::CppTPG* getTPG(QString id) {
    if (id.compare(QString::fromAscii(myID)) == 0)
        return new Cam::CppInverseKinematicTPG();
    return 0;
}

/// TPG Implementation ///
namespace Cam {


CppInverseKinematicTPG::CppInverseKinematicTPG()
        : CppTPG() { // important to call parent constructor
    id = QS(myID);
    name = QS(myName);
    description = QS(myDesc);

    this->toolpath = NULL;

    QString qaction = QS("default");
    actions.push_back(qaction);

    //TODO: create the 3 axis TPG we are using
}

CppInverseKinematicTPG::~CppInverseKinematicTPG() {
	//TODO: free the 3 axis TPG we are using
}


/* virtual */ void CppInverseKinematicTPG::initialise(TPGFeature *tpgFeature)
{
	CppTPG::initialise(tpgFeature);	// We must do this first so that we have somewhere to store our properties.

	QString qaction = QS("default");

	if (settings != NULL)
	{
	  //TODO: call initialise on the 3 axis TPG we are using
	}
}

/**
 * Run the TPG to generate the ToolPath code.
 *
 * Note: the return will change once the TP Language has been set in stone
 */
void CppInverseKinematicTPG::run(Settings::TPGSettings *settings, ToolPath *toolpath, QString action= QString::fromAscii(""))
{
    qDebug("CppInverseKinematicTPG running... \n");
    //TODO
    qDebug("CppInverseKinematicTPG done. \n");

}


/* virtual */ void CppInverseKinematicTPG::onChanged( Settings::Definition *tpgSettingDefinition, QString previous_value, QString new_value )
{
  //TODO: call onChanged on the 3 axis TPG we are using
	
	CppTPG::onChanged( tpgSettingDefinition, previous_value, new_value );
}


///* virtual */ ToolPath *CppInverseKinematicTPG::getToolPath()
//{
//	if (this->toolpath)
//	{
//		return(this->toolpath->grab());
//	}
//	else
//	{
//		return(NULL);
//	}
//}


} /* namespace Cam */
