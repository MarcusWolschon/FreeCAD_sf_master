/***************************************************************************
 *   Copyright (c) 2012 Luke Parry    (l.parry@warwick.ac.uk)              *
 *   Copyright (c) 2013 Andrew Robinson <andrewjrobinson@gmail.com>        *
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

#include <boost/bind.hpp>

#include <App/Application.h>
#include <App/Document.h>
#include <App/PropertyContainer.h>

#include <Base/Console.h>
#include <Base/Reader.h>
#include <Base/Writer.h>

#include "MachineFeature.h"

using namespace Cam;

PROPERTY_SOURCE(Cam::MachineFeature, Cam::CamSettingsableFeature)

MachineFeature::MachineFeature()
{
	//ADD_PROPERTY_TYPE(_prop_, _defaultval_, _group_,_type_,_Docu_)
    ADD_PROPERTY_TYPE(MachineId,        (""),  "Machine Feature", (App::PropertyType)(App::Prop_ReadOnly) , "Machine ID");
}

MachineFeature::~MachineFeature()
{
//	delObjConnection.disconnect();
}

App::DocumentObjectExecReturn *MachineFeature::execute(void)
{
    return App::DocumentObject::StdReturn;
}

void MachineFeature::onSettingDocument()
{
    //Create a signal to observe slot if this item is deleted
//    delObjConnection = getDocument()->signalDeletedObject.connect(boost::bind(&Cam::MachineFeature::onDelete, this, _1));
}

/**
	Called by the App::Property framework just before a property is changed.
 */
//void MachineFeature::onBeforeChange(const App::Property* prop)
//{
//}

//void MachineFeature::onChanged(const App::Property* prop)
//{
//}

void MachineFeature::initialise()
{
	return;
}

void MachineFeature::onDelete(const App::DocumentObject &docObj) {

//    // If deleted object me, proceed to delete my children
//    const char *myName = getNameInDocument();
//    if(myName != 0 && std::strcmp(docObj.getNameInDocument(), myName) == 0) {
//        App::Document *pcDoc = getDocument();
//
//        // remove the toolpath object if needed
//        if (this->ToolPath.getValue() != NULL) {
//            pcDoc->remObject(this->ToolPath.getValue()->getNameInDocument());
//        }
//    }
}

//void MachineFeature::Save(Base::Writer &writer) const
//{
//    //save the father classes
//    App::DocumentObject::Save(writer);
//}

//void MachineFeature::Restore(Base::XMLReader &reader)
//{
//    //read the father classes
//    App::DocumentObject::Restore(reader);
//}

//void MachineFeature::onDocumentRestored()
//{
//}
