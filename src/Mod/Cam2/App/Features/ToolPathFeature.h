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

#ifndef CAM_TOOLPATHFEATURE_H
#define CAM_TOOLPATHFEATURE_H

#include <PreCompiled.h>

namespace Cam {
class CamExport ToolPathFeature;
}

#include <App/DocumentObject.h>
#include <App/PropertyLinks.h>
#include <App/PropertyStandard.h>
#include <Mod/Part/App/PartFeature.h>
#include <Base/BoundBox.h>

//#include "../TPG/TPG.h"
//#include "../TPG/TPGSettings.h"
//#include "../TPG/TPGFactory.h"
#include "../TPG/ToolPath.h"

/**
 * ToolPathFeature is a wrapper for the ToolPath object within the Document Tree.
 */
namespace Cam
{
class CamExport ToolPathFeature : public App::DocumentObject
{
    PROPERTY_HEADER(Cam::ToolPathFeature);

public:
    ToolPathFeature();
    ~ToolPathFeature();

    App::PropertyStringList TPCommands;

    /// recalculate the Feature
    App::DocumentObjectExecReturn *execute(void);

    const char* getViewProviderName(void) const {
        return "CamGui::ViewProviderToolPathFeature";
    }

    void initialise() {};

    virtual void Save(Base::Writer &/*writer*/) const;
//    virtual void Restore(Base::XMLReader &/*reader*/);

    void setToolPath(ToolPath *toolpath);
    ToolPath* getToolPath();

protected:
    ToolPath *toolPath;
    
    virtual void onDocumentRestored();
};

} //namespace Cam


#endif //CAM_TOOLPATHFEATURE_H
