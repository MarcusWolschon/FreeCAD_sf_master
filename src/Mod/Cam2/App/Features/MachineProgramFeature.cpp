/***************************************************************************
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

#include "MachineProgramFeature.h"
#include "../MachineProgram.h"

using namespace Cam;

PROPERTY_SOURCE(Cam::MachineProgramFeature, App::DocumentObject)

MachineProgramFeature::MachineProgramFeature() {
    ADD_PROPERTY_TYPE(MPCommands,(""),"MachineProgram",App::Prop_None,"The list of commands that make up the Machine Program");
    machineProgram = NULL;
}

MachineProgramFeature::~MachineProgramFeature()
{
}

App::DocumentObjectExecReturn *MachineProgramFeature::execute(void)
{
    return App::DocumentObject::StdReturn;
}

void MachineProgramFeature::Save(Base::Writer &writer) const
{
    //save the parent classes
    App::DocumentObject::Save(writer);
}

void MachineProgramFeature::setMachineProgram(MachineProgram *machineProgram) {
    if (this->machineProgram)
        this->machineProgram->release();
    this->machineProgram = machineProgram->grab();

    // copy the commands out of machine program and save in internal storage
    QStringList::const_iterator it;
    std::vector<std::string> result;
    for (it = machineProgram->getMachineProgram()->constBegin(); it != machineProgram->getMachineProgram()->constEnd(); ++it) {
        result.push_back((*it).toStdString());
    }
    this->MPCommands.setValues(result);
}
MachineProgram* MachineProgramFeature::getMachineProgram() {
    if (machineProgram == NULL) {
        machineProgram = new MachineProgram(MPCommands.getValues(), NULL);
    }
    return machineProgram;
}

void MachineProgramFeature::onDocumentRestored()
{
}
