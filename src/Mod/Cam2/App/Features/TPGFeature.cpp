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

#include "TPGFeature.h"
#include "../TPG/TPGFactory.h"

#include <App/Document.h>
#include <App/Application.h>

#include <boost/bind.hpp>

using namespace Cam;

PROPERTY_SOURCE(Cam::TPGFeature, App::DocumentObject)

TPGFeature::TPGFeature()
{
	//ADD_PROPERTY_TYPE(_prop_, _defaultval_, _group_,_type_,_Docu_)
    ADD_PROPERTY_TYPE(PluginId,        (""),  "TPG Feature", (App::PropertyType)(App::Prop_ReadOnly) , "Plugin ID");
    ADD_PROPERTY_TYPE(PropTPGSettings, (),    "TPG Feature", (App::PropertyType)(App::Prop_None) , "TPG's Settings storage");
    ADD_PROPERTY_TYPE(ToolPath,        (0),   "TPG Feature", (App::PropertyType)(App::Prop_None),"ToolPath");
    ADD_PROPERTY_TYPE(MachineProgram,  (0),   "TPG Feature", (App::Prop_None),"MachineProgram");

    tpg = NULL;
    tpgSettings = new Settings::TPGSettings;
	tpgSettings->setTPGFeature(this);
}


//// TODO not sure if this is actually needed anymore.
//TPGFeature::TPGFeature(TPGDescriptor *tpgDescriptor)
//{
////    loadTPG(tpgDescriptor);
//}

//bool TPGFeature::loadTPG(TPGDescriptor *tpgDescriptor)
//{
//    if(tpgDescriptor == NULL)
//        throw Base::Exception("TPG Plugin Description is null");
//
//    // First check if a plugin already exists and quit if already running
//    if(this->hasRunningTPG())
//        return false;
//
//    // Make the plugin from the descriptor
//    TPG *myTpg = NULL;
//    myTpg = tpgDescriptor->make();
//
//    if(myTpg == NULL)
//        return false;
//
//    // Set the TPGFeatures internal TPG member
//    this->tpg = myTpg;
//
//    // Set the PluginID Property
//    PluginId.setValue(myTpg->getId().toStdString());
//    return true;
//}
//
//void TPGFeature::run()
//{
//      // Load the TPG, this
//     if(hasTPG()/*&& tpg->isReady()*/) {
//       //Initialise the TPG for running
////        this->tpg->setInputBBox(inputBBox); // Set the bounding box could be null
//      std::stringstream ss;
//      ss << "Running : " << this->tpg->getName().toStdString() << "\n";
//      Base::Console().Log(ss.str().c_str());
//       //this->tpg->run();
//   } else
//       Base::Console().Log("The TPG is not ready");
//}
//
//void TPGFeature::stop()
//{
////    if(hasRunningTPG())
////        this->tpg->stop();
////    else
////        Base::Console().Log("The TPG is not running");
//}
//
//TPG::State TPGFeature::getTPGStatus() {
////    return (hasTPG()) ?  this->tpg->getState() : TPG::UNDEFINED;
//	return TPG::UNDEFINED;
//}
//
//bool TPGFeature::hasRunningTPG()
//{
//    TPG::State tpgStatus = getTPGStatus();
//    return (tpgStatus == TPG::RUNNING || tpgStatus == TPG::STARTED);
//}

TPGFeature::~TPGFeature()
{
//    // If the TPG is running we must stop this (especially if a (network) process or thread
//    if(this->hasRunningTPG())
//        stop();
//    //TODO should we wait till the tpg has finished
//    tpg->release(); // Will internally call destructor and safely stop this

	delObjConnection.disconnect();

	if (tpg != NULL)
	{
		tpg->release();
		tpg = NULL;
	}

	if (tpgSettings != NULL)
	{
		tpgSettings->release();
		tpgSettings = NULL;
	}


}
void TPGFeature::onSettingDocument()
{
    //Create a signal to observe slot if this item is deleted
    delObjConnection = getDocument()->signalDeletedObject.connect(boost::bind(&Cam::TPGFeature::onDelete, this, _1));

    //test
    qDebug("setting TPG  Feature Document");

}

App::DocumentObjectExecReturn *TPGFeature::execute(void)
{
    Base::Console().Log("Running Feature \n");
//    this->run();
    return App::DocumentObject::StdReturn;
}

/**
	Called by the App::Property framework just before a property is changed.
	We want this because our PropTPGSettings member is a map of string properties.
	We can only figure out which of the properties embedded within the
	PropTPGSettings map changed by comparing the old and new maps.
 */
void TPGFeature::onBeforeChange(const App::Property* prop)
{
	if (prop == &PropTPGSettings)
	{
		const App::PropertyMap *property_map = dynamic_cast<const App::PropertyMap *>(prop);
		if (property_map)
		{
			// Let the tpgSettings object know that something is about to change.
			if (tpgSettings != NULL)
			{
				tpgSettings->onBeforePropTPGSettingsChange(property_map);
			}
		}
	}
}


/**
	Figure out which of our property types changed and signal the underlying
	TPGSettings object accordingly.

	This method is called by the App::Property framework automatically just
	after a property has changed.

	It's possible that we store some settings in a member variable OTHER than
	the PropTPGSettings member.  If that's the case then this method is the
	place where the association is made.  i.e. we need to figure out which
	member variable holds the modified setting and signal the underlying
	TPGSettings object accordingly.
 */
void TPGFeature::onChanged(const App::Property* prop)
{
	if (prop == &PropTPGSettings)
	{
		const App::PropertyMap *property_map = dynamic_cast<const App::PropertyMap *>(prop);
		if (property_map)
		{
			// Let the tpgSettings object know that something changed.
			if (tpgSettings != NULL)
			{
				tpgSettings->onPropTPGSettingsChanged(property_map);
			}
		}
	}
}



/**
 * Get a reference to the TPG that implements this TPG Feature.
 * Will load the TPG instance if not set already.
 */
TPG* TPGFeature::getTPG() {
	if (tpg == NULL)
	{
		tpg = TPGFactory().getPlugin(QString::fromStdString(PluginId.getStrValue()));
		if (tpg != NULL)
		{
			tpg->initialise(this);
		}
	}

	return tpg;
}

/**
 * NOTE: It's important that we do NOT initialise the TPG within this initialise() method.
 * This is due to the delayed loading of TPG objects.  It's possible that the TPGFeature
 * object will be instantiated and initialised by virtue of having been found in the
 * document file.  We may not yet have the TPG object loaded into memory.  Leave the
 * initialisation to the getTPG() method instead.
 */
void TPGFeature::initialise()
{
	return;
}

/**
 * Get the current TPG settings object
 */
Settings::TPGSettings* TPGFeature::getTPGSettings() {
	getTPG();	// Try to load and initialise the TPG so that our settings array is fully populated.
	return tpgSettings;
}

void TPGFeature::onDelete(const App::DocumentObject &docObj) {

    // If deleted object matches this cam feature, proceed to delete children
    const char *myName = getNameInDocument();
    if(myName != 0 && std::strcmp(docObj.getNameInDocument(), myName) == 0) {
        App::Document *pcDoc = getDocument();

        // remove the toolpath object if needed
        if (this->ToolPath.getValue() != NULL) {
            pcDoc->remObject(this->ToolPath.getValue()->getNameInDocument());
        }
    }
}

void TPGFeature::Save(Base::Writer &writer) const
{
	//NOTE: this isn't need anymore as the setting values are placed directly into the PropTPGSettings object
//	// save the settings into the property before saving
//	if (tpgSettings != NULL) {
//		std::map<std::string,std::string> newVals;
//		std::vector<TPGSettingDefinition*> settings = tpgSettings->getSettings();
//		for (std::vector<TPGSettingDefinition*>::iterator it = settings.begin(); it != settings.end(); ++it) {
//			std::string name = (*it)->name.toStdString();
//			std::string value = (*it)->value.toStdString();
//			newVals[name] = value;
//		}
////		PropTPGSettings.setValues(newVals);
//	}

    //save the father classes
    App::DocumentObject::Save(writer);
}

//void TPGFeature::Restore(Base::XMLReader &reader)
//{
//    //read the father classes
//    App::DocumentObject::Restore(reader);
//}


/**
 * Set the toolpath object for this TPG.
 */
void TPGFeature::setToolPath(ToolPathFeature *toolPath) {
    if (this->ToolPath.getValue() != NULL) {
        App::Document *pcDoc = getDocument();
        pcDoc->remObject(this->ToolPath.getValue()->getNameInDocument());
    }
    this->ToolPath.setValue(toolPath);
}

/**
 * Set the machine program object for this TPG.
 */
void TPGFeature::setMachineProgram(Cam::MachineProgram *machineProgram) {

    // get the Active document
    App::Document* doc = getDocument();
    if (!doc) {
        Base::Console().Error("No document! Please create or open a FreeCad document\n");
        return;
    }

    // construct a name for toolpath
    const char *tpgname = getNameInDocument();
    std::string toolpathName;
    toolpathName.append(tpgname);
    toolpathName.append("-MachineProgram");
    std::string mpFeatName = doc->getUniqueObjectName(toolpathName.c_str());

    // create the feature (for Document Tree)
    App::DocumentObject *machineProgramFeat =  doc->addObject("Cam::MachineProgramFeature", mpFeatName.c_str());
    if(machineProgramFeat && machineProgramFeat->isDerivedFrom(Cam::MachineProgramFeature::getClassTypeId())) {
        Cam::MachineProgramFeature *machineProgramFeature = dynamic_cast<Cam::MachineProgramFeature *>(machineProgramFeat);

        machineProgramFeature->initialise();

        // wrap machine program object in mpfeature
        machineProgramFeature->setMachineProgram(machineProgram);

        // add mpfeature to tpg
        if (this->MachineProgram.getValue() != NULL) {
            App::Document *pcDoc = getDocument();
            pcDoc->remObject(this->MachineProgram.getValue()->getNameInDocument());
        }
        this->MachineProgram.setValue(machineProgramFeature);

        doc->recompute();
        qDebug("Added ToolPath");
    }
    else
        qDebug("Unable to create MachineProgramFeature: %p", machineProgramFeat);
}

void TPGFeature::onDocumentRestored()
{
    // Attempt to Load the plugin - guarantee no TPG is loaded
    // TODO not sure if we should catch this higher up but this is the only place
//    try {
//      std::string id = PluginId.getValue();
//      if(id.length() > 0) {
//        // Load the TPG Plugin
//        TPG *tpgPlugin = TPGFactory().getPlugin(QString::fromStdString(id));
//        if(tpgPlugin == NULL)
//          throw Base::Exception("Plugin couldn't be loaded");
//
//        this->tpg = tpgPlugin;
//      }
//    } catch (...) {
//
//    }

	this->tpgSettings->addDefaults();
}
