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

//#define CamGuiExport
#include "PreCompiled.h"

#ifndef CAMGUI_VIEWPROVIDERMACHINEPROGRAMFEATURE_H
#define CAMGUI_VIEWPROVIDERMACHINEPROGRAMFEATURE_H

#include <Gui/ViewProviderDocumentObject.h>
#include "../App/Features/MachineProgramFeature.h"

class QMenu;

namespace Gui
{
  class View3DInventorViewer;
}


namespace Cam 
{
  class TPGFeature;
}

namespace CamGui {

class CamGuiExport ViewProviderMachineProgramFeature : public Gui::ViewProviderDocumentObject
{
    PROPERTY_HEADER(CamGui::ViewProviderMachineProgramFeature);

public:
    /// constructor
    ViewProviderMachineProgramFeature();
    /// destructor
    virtual ~ViewProviderMachineProgramFeature();

    void setupContextMenu(QMenu *menu, QObject *receiver, const char *member);
    bool setEdit(int ModNum);
    void setEditViewer(Gui::View3DInventorViewer* viewer, int ModNum);
    
    void unsetEditViewer(Gui::View3DInventorViewer* viewer);
    void unsetEdit(int ModNum);
    bool doubleClicked(void);

    Cam::MachineProgramFeature* getObject() const;

    QIcon getIcon(void) const;
};

} // namespace


#endif //CAMGUI_VIEWPROVIDERTOOLPATHFEATURE_H
