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
# include <TopoDS_Shape.hxx>
# include <TopoDS_Face.hxx>
# include <TopoDS.hxx>
# include <TopExp_Explorer.hxx>
# include <BRepAdaptor_Surface.hxx>
# include <BRepAdaptor_Curve.hxx>
# include <BRep_Tool.hxx>
# include <Geom_Plane.hxx>
# include <GeomAPI_ProjectPointOnSurf.hxx>
# include <BRepOffsetAPI_NormalProjection.hxx>
# include <BRepBuilderAPI_MakeFace.hxx>
#endif

#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>

#include <Base/Writer.h>
#include <Base/Reader.h>
#include <Base/Tools.h>
#include <Base/Console.h>
#include <Base/Exception.h>

#include <vector>

#include <boost/bind.hpp>

//#include "CamPartsList.h"
//#include "TPGList.h"
//#include "StockGeometry.h"
//#include "GCodeFeature.h"

#include "CamFeature.h"

// #include "CamFeaturePy.h"

using namespace Cam;
using namespace Base;

PROPERTY_SOURCE(Cam::CamFeature, App::DocumentObject)

const char *camFeatGroup = "Cam Feature";

CamFeature::CamFeature()
{
    ADD_PROPERTY_TYPE(TPGList,(0), camFeatGroup, (App::PropertyType)(App::Prop_None) ,"TPG Features");
//    ADD_PROPERTY_TYPE(MachineProgramList,(0), camFeatGroup, (App::PropertyType)(App::Prop_None) ,"Machine Program Features");

//    ADD_PROPERTY_TYPE(Result,              (0), camFeatGroup, (App::Prop_None),"Result");
//    ADD_PROPERTY_TYPE(TPGListLink,         (0), camFeatGroup, (App::Prop_None),"TPGList");
//    ADD_PROPERTY_TYPE(StockGeometryObject ,(0), camFeatGroup, (App::PropertyType)(App::Prop_None) ,"Stock Geometry");
//    ADD_PROPERTY_TYPE(CamPartsListObject  ,(0), camFeatGroup, (App::PropertyType)(App::Prop_None) ,"Cam Parts List");
}

CamFeature::~CamFeature()
{
    delObjConnection.disconnect();
}

void CamFeature::onSettingDocument()
{
    //Create a signal to observe slot if this item is deleted
    delObjConnection = getDocument()->signalDeletedObject.connect(boost::bind(&Cam::CamFeature::onDelete, this, _1));

    //test

}


/**
 * Initialise creates the basic Cam Feature tree with all the required sub features
 */
void CamFeature::initialise()
{    
    App::Document *doc = getDocument();

//    //Generate unique names for the objects
//    std::string CamPartsListName  = doc->getUniqueObjectName("CamPartsList");
//    std::string StockGeometryName = doc->getUniqueObjectName("StockGeometry");
//    std::string TPGListName       = doc->getUniqueObjectName("ToolPaths");
//    std::string GCodeFeatName     = doc->getUniqueObjectName("GCodeFeature");
//
//    // Create the SubNodes that will be referenced by pointers for quick access;
//
//    App::DocumentObject *camPartsObj  = doc->addObject("Cam::CamPartsList" , CamPartsListName.c_str());
//    App::DocumentObject *stockGeomObj = doc->addObject("Cam::StockGeometry", StockGeometryName.c_str());
//    App::DocumentObject *TPGListObj   = doc->addObject("Cam::TPGList"      , TPGListName.c_str());
//    App::DocumentObject *gcodeFeatObj = doc->addObject("Cam::GCodeFeature" , GCodeFeatName.c_str());
//
//    if(!camPartsObj || !camPartsObj->isDerivedFrom(CamPartsList::getClassTypeId()))
//        throw Base::Exception("A CamPartsList Feature couldn't successfuly be created");
//
//    if(!stockGeomObj || !stockGeomObj->isDerivedFrom(StockGeometry::getClassTypeId()))
//        throw Base::Exception("A Stock Geometry Feature couldn't successfuly be created");
//
//    if(!TPGListObj || !TPGListObj->isDerivedFrom(TPGList::getClassTypeId()))
//        throw Base::Exception("A TPGList Feature couldn't successfuly be created");
//
//    TPGList *tpgList = dynamic_cast<TPGList *>(TPGListObj);
//    CamPartsListObject.setValue(camPartsObj); // Create Object Link
//    StockGeometryObject.setValue(stockGeomObj); // Create Object Link
//
//    if(!gcodeFeatObj || !gcodeFeatObj->isDerivedFrom(GCodeFeature::getClassTypeId()))
//        throw Base::Exception("A GCode Feature couldn't successfuly be created");
//
//    GCodeFeature *gCodeFeat = dynamic_cast<GCodeFeature *>(gcodeFeatObj);
////     gCodeFeat->TPGListObj.setValue(TPGListObj); // Create Object Link
//    TPGListLink.setValue(TPGListObj); // Create Object Link
//    this->Result.setValue(gcodeFeatObj);

//    doc->recompute();
}

void CamFeature::onDelete(const App::DocumentObject &docObj) {

    // If deleted object matches this cam feature, proceed to delete children
	const char *myName = getNameInDocument();
    if(myName != 0 && std::strcmp(docObj.getNameInDocument(), myName) == 0) {
        App::Document *pcDoc = getDocument();

        // remove the children TPG's
        const std::vector<App::DocumentObject *> vals = TPGList.getValues();
		for (std::vector<App::DocumentObject *>::size_type i = 0; i < vals.size(); i++)
			pcDoc->remObject(vals[i]->getNameInDocument());

        // remove the children Machine code objects's
//		const std::vector<App::DocumentObject *> vals2 = MachineProgramList.getValues();
//		for (int i = 0; i < vals2.size(); i++)
//			pcDoc->remObject(vals2[i]->getNameInDocument());
    }
}

short int CamFeature::mustExecute() const
{
//    if(StockGeometryObject.isTouched() || CamPartsListObject.isTouched())
//        return 1;
    return App::DocumentObject::mustExecute();
}


App::DocumentObjectExecReturn *CamFeature::execute(void)
{
  
//    TPGList *tpgList = getTPGContainer();
//    if(tpgList)
//    {
////         tpgList->setCamPartsList(getPartsContainer());
////         tpgList->setStockGeometry(getStockGeometry());
//         tpgList->execute();
//    }
    return App::DocumentObject::StdReturn;
}

// PyObject *CamFeature::getPyObject(void)
// {
//     if (PythonObject.is(Py::_None())) {
//         // ref counter is set to 1
//         PythonObject = Py::Object(new CamFeaturePy(this),true);
//     }
//     return Py::new_reference_to(PythonObject);
// }

//TPGList * CamFeature::getTPGContainer() const
//{
////    App::DocumentObject *obj = TPGListLink.getValue();
////    if(obj && obj->isDerivedFrom(TPGList::getClassTypeId()))
////      return dynamic_cast<TPGList *>(obj);
////    else
//      return 0;
//}
//
//
//StockGeometry * CamFeature::getStockGeometry() const
//{
//    StockGeometry * stockGeom = 0;
////    App::DocumentObject * docObj = StockGeometryObject.getValue();
////    if(docObj && docObj->isDerivedFrom(StockGeometry::getClassTypeId()))
////        stockGeom = dynamic_cast<StockGeometry *>(docObj);
//
//    return stockGeom;
//}
//
//CamPartsList * CamFeature::getPartsContainer() const
//{
//    CamPartsList * camParts= 0;
////    App::DocumentObject * docObj = CamPartsListObject.getValue();
////    if(docObj && docObj->isDerivedFrom(CamPartsList::getClassTypeId()))
////        camParts = dynamic_cast<CamPartsList *>(docObj);
//
//    return camParts;
//}
//
//GCodeFeature * CamFeature::getGCodeFeature() const
//{
//    GCodeFeature *feat = NULL;
////    App::DocumentObject *docObj = Result.getValue();
////    if(docObj && docObj->isDerivedFrom(GCodeFeature::getClassTypeId()))
////        feat = dynamic_cast<GCodeFeature *>(docObj);
//
//    return feat;
//}

unsigned int CamFeature::getMemSize(void) const
{
    return 0;
}

void CamFeature::addTPG(TPGFeature *tpg)
{
	const std::vector<App::DocumentObject *> vals = TPGList.getValues();
	std::vector<App::DocumentObject *> newVals(vals);
	newVals.push_back(tpg);
	TPGList.setValues(newVals);
}

void CamFeature::Save(Writer &writer) const
{
    //save the father classes
    App::DocumentObject::Save(writer);
}

void CamFeature::Restore(XMLReader &reader)
{
    //read the father classes
    App::DocumentObject::Restore(reader);
}
