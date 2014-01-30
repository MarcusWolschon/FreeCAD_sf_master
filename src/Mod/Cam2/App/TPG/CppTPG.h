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

#ifndef CPPTPG_H_
#define CPPTPG_H_

#include <QString>

#include "TPG.h"
#include "TPGDescriptorCollection.h"

// Forward declarations
namespace Cam {

class CppTPG;

// Descriptor create function types
typedef Cam::TPGDescriptorCollection* getDescriptors_t();

// TPG create function types
typedef CppTPG* getTPG_t(QString);
}

#include "CppTPGPlugin.h"

/// Macros to make it simpler to define a new CppTPG ///
/**
 * Support macros
 */
#define QS(_cs_) QString::fromAscii(_cs_)
#define _TPGDescriptor(_type_, _id_, _name_, _desc_) new Cam::CppTPGDescriptor(QS(_id_), QS(_name_), QS(_desc_))
#define _MakeTPG(_type_, _id_, _name_, _desc_) if (id == QString::fromAscii(_id_))\
        return new _type_();\

/**
 * TPG Plugins that implement a single plugin should add this macro to its
 * source file
 * TODO: see if its possible to make a nice macro that can handle multiple TPG's at once
 */
#define CPPTPG_API_SOURCE(_type_, _id_, _name_, _desc_)\
extern "C" CamExport Cam::TPGDescriptorCollection* getDescriptors() {\
    Cam::TPGDescriptorCollection* descriptors = new Cam::TPGDescriptorCollection();\
    Cam::TPGDescriptor *descriptor = _TPGDescriptor(_type_, _id_, _name_, _desc_);\
    descriptors->add(descriptor);\
    descriptor->release();\
    return descriptors;\
}\
extern "C" CamExport Cam::CppTPG* getTPG(QString id) {\
    _MakeTPG(_type_, _id_, _name_, _desc_)\
    return NULL;\
}

/**
 * TPG Plugins that implement a single plugin should add this macro to its
 * header file
 */
#define CPPTPG_API_HEADER()\
extern "C" CamExport Cam::TPGDescriptorCollection* getDescriptors();\
extern "C" CamExport Cam::CppTPG* getTPG(QString);

namespace Cam {


class CamExport CppTPG : public TPG {

protected:
    CppTPGPlugin* plugin;


    CppTPG();
    ~CppTPG();

public:
    /**
     * Set the plugin that was used to create this create this instance.
     *
     * This is so it can obtain a reference to it and thus cause the plugin to
     * stay open until this instance is released.
     */
    void setPlugin(CppTPGPlugin* plugin);

	/*
	 * Inherited from TPG::initialise(TPGFeature *tpgFeature)
	 */
	virtual void initialise(TPGFeature *tpgFeature);

	/*
	 * Inherited from the TPG::onChanged() method.
	 */
	virtual void onChanged( Settings::Definition *tpgSettingDefinition, QString previous_value, QString new_value);
};


}



#endif /* CPPTPG_H_ */
