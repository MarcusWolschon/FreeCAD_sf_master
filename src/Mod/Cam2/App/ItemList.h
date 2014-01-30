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

#ifndef ITEMLIST_H_
#define ITEMLIST_H_

#include <vector>

#include <QString>

namespace Cam {

/**
 * An Item that can go in the ItemList.  It stores a name and its id
 */
class Item {
protected:
    QString id;
    QString name;

    int refcnt;

public:
    Item() {
        refcnt = 1;
    }
    Item(int id, QString name) {
        this->id = id;
        this->name = name;
        refcnt = 1;
    }

    QString getId() const {
        return id;
    }
    void setId(QString id) {
        this->id = id;
    }

    QString getName() const {
        return name;
    }
    void setName(QString name) {
        this->name = name;
    }

    /**
     * Increases reference count
     * Note: it returns a pointer to this for convenience.
     */
    Item *grab() {
        refcnt++;
        return this;
    }

    /**
     * Decreases reference count and deletes self if no other references
     */
    void release() {
        refcnt--;
        if (refcnt == 0)
            delete this;
    }
};

class ItemList {
protected:
    std::vector<Item*> items;

    int refcnt;

public:
    ItemList();
    virtual ~ItemList();

    /**
     * Increases reference count
     * Note: it returns a pointer to this for convenience.
     */
    ItemList *grab() {
        refcnt++;
        return this;
    }

    /**
     * Decreases reference count and deletes self if no other references
     */
    void release() {
        refcnt--;
        if (refcnt == 0)
            delete this;
    }
};

} /* namespace Cam */
#endif /* ITEMLIST_H_ */
