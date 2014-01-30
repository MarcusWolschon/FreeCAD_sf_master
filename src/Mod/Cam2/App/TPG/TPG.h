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


#ifndef CAM_TPG_H
#define CAM_TPG_H

#include <vector>
#include <map>

#include <QString>
#include <vector>
#include <map>

#include <Base/Type.h>
#include <Base/BaseClass.h>
#include <Base/BoundBox.h>

namespace Cam {
class CamExport TPG;
}
//#include "../GCode.h"
//#include "TPGCache.h"
#include "ToolPath.h"
#include "TPGSettings.h"
#include "../Features/TPGFeature.h"

namespace Cam
{

class CamExport TPGFeature;
//class CamExport TPGSettings;
//class TPG;
#ifdef ERROR
	#undef ERROR	// This macro name is included in Windows standard header files so it doesn't compile on Windows builds.
#endif

class CamExport TPG : public Base::BaseClass
{
    TYPESYSTEM_HEADER();
public:
    enum State {
      UNDEFINED,
      LOADED,
      INITIALISED,
      STARTED,
      RUNNING,
      ERROR,
      FINISHED
    };
    enum Type {
      PLUGIN_UNDEFINED,
      PLUGIN_LIB,
      PLUGIN_NETWORK,
      PLUGIN_PROCESS,
      PLUGIN_PYTHON
    };

    static QString stateToStr(State state);

    /**
     * Default Constructor
     */
    TPG();
    
    /**
     * Complimentary constructor to assign plugin properties
     */
    TPG(const QString &TPGId, const QString &TPGName, const QString &TPGDescription);

    virtual void initialise(TPGFeature *tpgFeature);

	QString settingName_Geometry() const;	// Used by all TPG objects.
	QString settingName_Tool() const;

    // TPG API methods.  Used by the CAM workbench to run the TPG

    /**
     * Returns a list of action names supported by this TPG.
     *
     * Note: if this TPG only does one thing then just return a list containing 'default'
     */
    virtual std::vector<QString> &getActions();

    /**
     * Get the settings for a given action
     */
	virtual Settings::TPGSettings *getSettingDefinitions();

	/**
	 * Allow each TPG to be notied when one of the settings changes.
	 */
	virtual void onChanged( Settings::Definition *tpgSettingDefinition, QString previous_value, QString new_value);

    /**
     * Run the TPG to generate the ToolPath code.
     *
     * Note: the return will change once the TP Language has been set in store
     */
	virtual void run(Settings::TPGSettings *settings, ToolPath *toolpath, QString action);

//    /**
//     * Returns the toolpath from the last run
//     */
//    virtual ToolPath *getToolPath() = 0;

    virtual QString getId() { return id; }
    virtual QString getName() { return name; }
    virtual QString getDescription() { return description; }

    /**
     * Increases reference count
     */
    void grab();

    /**
     * Decreases reference count and deletes self if no other references
     */
    void release();

    /* [arobinson] Commented all this out until it is needed, it was making
         * things more complex when making the of the Python API
    virtual void stop();

	// Convenience method: the state must be explicitly set to intiailised even when it has finished. This ensures data structure is reconstructed
    bool isReady() { return (this->state == INITIALISED || this->state == FINISHED); }

    /// Setters
    void setInputBBox(const Base::BoundBox3d bbox) { inputBBox = bbox; }

    ///Output
    const Base::BoundBox3d & getOutputBBox() const { return outputBBox; }
    // Overridable Virtual Functions provide flexibility for different toolpaths generators
    virtual void getSTLModel();
    //virtual void getToolPath() = 0; // NOTE we need to devise the storage format for the TOOLPath output

    State getState() { return state; }

    /**
     * Get a list (vector<QString>) of action names this TPG supports.
     *
     * Passed by reference
     * /
    virtual std::vector<QString> &getActions()
	{
    	return this->actions;
	}

    virtual TPGSettings* getSettings(QString &action)
	{

	} */

//    virtual TPG* makeTPG();
protected:
    /* [arobinson] Commented all this out until it is needed, it was making
     * things more complex when making the of the Python API
    ///Internal Methods

    /// Internal - obtain the geometry in a TPG friendly format
    //getModelSTL(); // Use tpgFeat to get this
    //getModelVertArc(); // Use tpgFeat to get this

    /// The Bounding box must be calculated [TODO do we need this?]
    //virtual const Base::BoundBox3d & calculateBBox() = 0;

    /// Set the resultant bounding box
    const Base::BoundBox3d & setOutputBBox(const Base::BoundBox3d bbox) { outputBBox = bbox; }

    std::vector<QString> actions;
    std::map<QString, Cam::Settings::TPGSettings* > settings;
    TPGCache    *cache;
    State        state;
    */

    /**
     * Default Destructor
     *
     * (protected to force use of reference counting)
     */
    virtual ~TPG();

    // [mrlukeparry] Prefer to use QString because it gives UTF compatible strings
    QString id; // To gerenate a new id run import uuid; str(uuid.uuid1()); on the python console
    QString name;
    QString description;

    std::vector<QString> actions; ///< e.g ['action','action2',...]
//    std::map<QString, TPGSettings*> settings; ///< e.g. settings[<action>] // Action has been moved into TPGSettings object
    Settings::TPGSettings* settings; ///< the setting definitions for this TPG

	// Declare some pointers to individual settings definitions for ease of use.
	Settings::ObjectNamesForType *geometry;
	Settings::SingleObjectNameForType *tool;

    int refcnt; ///< reference counter

public:
	TPGFeature *tpgFeature;	// Pointer to parent.

//    // Storage of TPG Bounding Boxes
//    Base::BoundBox3d inputBBox;
//    Base::BoundBox3d outputBBox;
};

}
#endif //CAM_TPG_H
