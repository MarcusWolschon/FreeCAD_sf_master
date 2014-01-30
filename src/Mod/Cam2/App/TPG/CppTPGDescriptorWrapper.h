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

#ifndef CPPTPGDESCRIPTORWRAPPER_H_
#define CPPTPGDESCRIPTORWRAPPER_H_

#include "TPGFactory.h"
#include "TPG.h"
#include "CppTPGPlugin.h"
#include "TPGDescriptor.h"

namespace Cam {

//class TPG;

/**
 * A wrapper for the C++ TPGDescriptor objects.  This allows them to be deleted
 * correctly via the library.
 */
class CppTPGDescriptorWrapper: public TPGDescriptor {
protected:
    TPGDescriptor *descriptor;
    CppTPGPlugin *plugin;
    virtual ~CppTPGDescriptorWrapper();

public:
    CppTPGDescriptorWrapper(TPGDescriptor *descriptor, CppTPGPlugin *plugin=NULL);

    /**
     * Creates a new instance of this TPG.  Sub-classes need to implement this
     */
    virtual TPG* make();
};

} /* namespace CamGui */
#endif /* CPPTPGDESCRIPTORWRAPPER_H_ */
