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
#include <QMenu>
#endif

#include <Gui/Application.h>
#include <Gui/BitmapFactory.h>
#include <Gui/Command.h>
#include <Gui/Control.h>
#include <Gui/Document.h>
#include <Gui/SoFCSelection.h>
#include <Gui/Selection.h>
#include <Gui/SoTextLabel.h>
#include <Gui/MainWindow.h>
#include <Gui/SoFCUnifiedSelection.h>
#include <Gui/SoFCBoundingBox.h>
#include <Gui/View3DInventor.h>

#include "ViewProviderMachineProgramFeature.h"

using namespace CamGui;

PROPERTY_SOURCE(CamGui::ViewProviderMachineProgramFeature, Gui::ViewProviderDocumentObject)

ViewProviderMachineProgramFeature::ViewProviderMachineProgramFeature()
{
}

ViewProviderMachineProgramFeature::~ViewProviderMachineProgramFeature()
{
}

void ViewProviderMachineProgramFeature::setupContextMenu(QMenu *menu, QObject *receiver, const char *member)
{
    menu->addAction(QObject::tr("Edit ") + QString::fromAscii(getObject()->Label.getValue()), receiver, member);
}

bool ViewProviderMachineProgramFeature::setEdit(int ModNum)
{
    return FALSE;
}

void ViewProviderMachineProgramFeature::setEditViewer(Gui::View3DInventorViewer* viewer, int ModNum)
{
    viewer->setEditing(TRUE);
}

bool ViewProviderMachineProgramFeature::doubleClicked(void)
{
    Gui::Application::Instance->activeDocument()->setEdit(this);
    return true;
}

void ViewProviderMachineProgramFeature::unsetEditViewer(Gui::View3DInventorViewer* viewer)
{
    viewer->setEditing(FALSE);
}

void ViewProviderMachineProgramFeature::unsetEdit(int ModNum)
{
    // clear the selection and set the new/edited sketch(convenience)
    Gui::Selection().clearSelection();
}

Cam::MachineProgramFeature* ViewProviderMachineProgramFeature::getObject() const
{
    return dynamic_cast<Cam::MachineProgramFeature*>(pcObject);
}

QIcon ViewProviderMachineProgramFeature::getIcon(void) const
{
	return Gui::BitmapFactory().pixmap("Cam_MachineProgram");
}
