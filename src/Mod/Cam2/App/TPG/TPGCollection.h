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

#ifndef TPGCOLLECTION_H_
#define TPGCOLLECTION_H_

#include <vector>

#include "TPG.h"

namespace Cam {


/**
 * A reference counted collection of TPG's
 */
class TPGCollection {
protected:
    std::vector<TPG*> tpgs;

    int refcnt;

    /**
     * Protected destructor to force use of refcount functions
     */
    virtual ~TPGCollection();

public:
    TPGCollection();

    /**
     * Increases reference count
     */
    void grab();

    /**
     * Decreases reference count and deletes self if no other references
     */
    void release();

    /**
     * Adds a TPG to the collection
     */
    void add(TPG* tpg);

    /**
     * Absorbs the TPGs from the given collection.
     *
     * The other collection will be emptied but not released.
     */
    void absorb(TPGCollection &other);

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
    TPG* at(size_t pos);
};

} /* namespace Cam */
#endif /* TPGCOLLECTION_H_ */
