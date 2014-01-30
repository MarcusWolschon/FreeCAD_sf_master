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

#ifndef TPGDESCRIPTORCOLLECTION_H_
#define TPGDESCRIPTORCOLLECTION_H_

#include <PreCompiled.h>	// for CamExport macro definition.

namespace Cam {
class CamExport TPGDescriptorCollection;
}

#include <vector>

#include "TPG.h"
#include "TPGDescriptor.h"

namespace Cam {

class CamExport TPGDescriptorCollection {
public:
    TPGDescriptorCollection();

    /**
     * Increases reference count
     * Note: it returns a pointer to this for convenience.
     */
    TPGDescriptorCollection *grab();

    /**
     * Decreases reference count and deletes self if no other references
     */
    void release();

    /**
     * Adds a TPGDescriptor to the collection
     */
    void add(TPGDescriptor* descriptor);

    /**
     * Absorbs the TPGDescriptors from the given collection.
     *
     * The other collection will be emptied but not released.
     */

    void absorb(TPGDescriptorCollection *other);

    /**
     * Removes a TPG from the collection
     */
//    void del(TPG* tpg);

    /**
     * Get the number of items in this collection
     */
    size_t size();

    /**
     * Get the TPG at the given position
     */
    TPGDescriptor* at(size_t pos);

    /**
     * Makes a duplicate copy of this collection.  It will create a new object
     * and grab a new reference to each TPGDescriptor.
     */
    TPGDescriptorCollection* clone();

    /**
     * Calls a function for each element of in collection and assigns the element to is returned value
     */
    void onEach(void*func(TPGDescriptor*, void*), void *param);

    void print() {
        qDebug("TPGDescriptorCollection: (refs: %i)\n", refcnt);
    }

protected:
    int refcnt;
    std::vector<TPGDescriptor*> descriptors;

    virtual ~TPGDescriptorCollection();
};

} /* namespace Cam */
#endif /* TPGDESCRIPTORCOLLECTION_H_ */
