/***************************************************************************
 *   Copyright (c) 2012 Andrew Robinson <andrewjrobinson@gmail.com>        *
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

#ifndef CPPINVERSEKINEMATICTPG_H_
#define CPPINVERSEKINEMATICTPG_H_

#include <vector>

#include <TPG/CppTPG.h>
#include <TPG/TPGFactory.h>
#include <TPG/TPGSettings.h>
#include <TPG/ToolPath.h>

namespace Cam {

/**
 * This is an UNFINISHED TPG that postprocesses the output of
 * another TPG.
 * It takes the output of a 3 axis strategy to use the 4th axis
 * and inverse kinematic for following the surace normal (as close as
 * possible without colliding).
 */
class CppInverseKinematicTPG : public CppTPG {
protected:
    virtual ~CppInverseKinematicTPG();

public:
    CppInverseKinematicTPG();

    /**
     * Run the TPG to generate the ToolPath code.
     *
     * Note: the return will change once the TP Language has been set in stone
     */
    virtual void run(Settings::TPGSettings *settings, ToolPath *toolpath, QString action);

//    /**
//     * Returns the toolpath from the last
//     */
//    virtual ToolPath *getToolPath();

	/**
	 * Add the settings required by this ToolPath Generator.  This method
	 * also calls the CppTPG::initialiseSettings() method so that those
	 * associated with all TPGs (such as Geometry and Tool) are included
	 */
	virtual void initialise(TPGFeature *tpgFeature);

	/**
	 * Allow this object to know when one of its settings has changed.  NOTE: It's important
	 * that, if implemented, this object also calls the CppTPG::onChanged() method so that
	 * it, in turn, can call the TPG::onChanged() method.  These other layers of classes
	 * might also be interested.
	 */
	virtual void onChanged( Settings::Definition *tpgSettingDefinition, QString previous_value, QString new_value );

public:

private:
	ToolPath *toolpath;	// cache of toolpath from last call of the run() method.
};

} /* namespace Cam */

#endif /* CPPINVERSEKINEMATICTPG_H_ */
