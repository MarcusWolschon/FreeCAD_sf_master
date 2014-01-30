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

#include <App/PropertyContainer.h>

#include <Base/Console.h>
#include <Base/Reader.h>
#include <Base/Writer.h>

#include "ToolPathFeature.h"
//#include "../TPG/TPGFactory.h"

using namespace Cam;

PROPERTY_SOURCE(Cam::ToolPathFeature, App::DocumentObject)

ToolPathFeature::ToolPathFeature() {
    ADD_PROPERTY_TYPE(TPCommands,(""),"ToolPath",App::Prop_None,"The list of commands that make up the toolpath");
    toolPath = NULL;
}

ToolPathFeature::~ToolPathFeature()
{
}

App::DocumentObjectExecReturn *ToolPathFeature::execute(void)
{
    return App::DocumentObject::StdReturn;
}

void ToolPathFeature::Save(Base::Writer &writer) const
{
    //save the parent classes
    App::DocumentObject::Save(writer);
}

void ToolPathFeature::setToolPath(ToolPath *toolpath) {
    if (this->toolPath != NULL)
        this->toolPath->release();
    this->toolPath = toolpath->grab();

    // copy the commands out of toolpath and save in internal storage
    QStringList::const_iterator it;
    std::vector<std::string> result;
    for (it = toolpath->toolpath->constBegin(); it != toolpath->toolpath->constEnd(); ++it) {
        result.push_back((*it).toStdString());
    }
    this->TPCommands.setValues(result);
}
/**
 * Get the toolpath object.  Returned reference is owned by caller
 */
ToolPath* ToolPathFeature::getToolPath() {
    if (toolPath == NULL) {
        toolPath = new ToolPath(TPCommands.getValues());
    }
    return toolPath->grab();
}

void ToolPathFeature::onDocumentRestored()
{
}
