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

#include "../PreCompiled.h"
#ifndef _PreComp_
#endif

#include "TPGCollection.h"

namespace Cam {

TPGCollection::TPGCollection() {
    this->refcnt = 1;
}

TPGCollection::~TPGCollection() {
    //TODO: make this thread-safe
    for (size_t i = 0; i < tpgs.size(); i++)
        tpgs.at(i)->release();
    tpgs.clear();
}



/**
 * Increases reference count
 */
void TPGCollection::grab() {
    this->refcnt++;
}

/**
 * Decreases reference count and deletes self if no other references
 */
void TPGCollection::release() {
    this->refcnt--;
    if (this->refcnt <= 0)
        delete this;
}

/**
 * Adds a TPG to the collection
 */
void TPGCollection::add(TPG* tpg) {
    tpg->grab();
    tpgs.push_back(tpg);
}

/**
 * Absorbs the TPGs from the given collection.
 *
 * The other collection will be emptied but not released.  NOTE: the reference
 * to the TPGs in other will be 'stolen'.
 */
void TPGCollection::absorb(TPGCollection &other) {
    //TODO: make this thread-safe
    size_t cnt = other.tpgs.size();
    for (size_t i = 0; i < cnt; i++) {
        tpgs.push_back(other.tpgs.at(i));
    }
    other.tpgs.clear();
}

/**
 * Removes a TPG from the collection
 */
//void TPGCollection::del(TPG* tpg);

/**
 * Get the number of items in this collection
 */
size_t TPGCollection::size() {
    return tpgs.size();
}

/**
 * Get the TPG at the given position
 */
TPG* TPGCollection::at(size_t pos) {
    if (pos >= 0 && pos < tpgs.size())
        return tpgs.at(pos);
    return NULL;
}


} /* namespace Cam */
