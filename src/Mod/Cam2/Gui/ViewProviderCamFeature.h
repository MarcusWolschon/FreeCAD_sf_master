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

#ifndef CAMGUI_VIEWPROVIDERCAMFEATURE_H
#define CAMGUI_VIEWPROVIDERCAMFEATURE_H

#include <Gui/ViewProviderDocumentObject.h>

class QMenu;

namespace Gui
{
  class View3DInventorViewer;
}

namespace Cam {
    class CamFeature;
}

namespace CamGui {
class CamGuiExport ViewProviderCamFeature : public Gui::ViewProviderDocumentObject
{
    PROPERTY_HEADER(CamGui::ViewProviderCamFeature);

public:
    /// constructor
    ViewProviderCamFeature();
    /// destructor
    virtual ~ViewProviderCamFeature();

    void setupContextMenu(QMenu *menu, QObject *receiver, const char *member);
    bool setEdit(int ModNum);
    void setEditViewer(Gui::View3DInventorViewer* viewer, int ModNum);
    
    void unsetEditViewer(Gui::View3DInventorViewer* viewer);
    void unsetEdit(int ModNum);
    bool doubleClicked(void);

    /// grouping handling
    std::vector<App::DocumentObject*> claimChildren(void)const;
    Cam::CamFeature* getObject() const;

//    std::vector<std::string> getDisplayModes(void) const;

//    QIcon getIcon(void) const;
};

} // namespace
#endif //CAMGUI_VIEWPROVIDERCAMFEATURE_H
