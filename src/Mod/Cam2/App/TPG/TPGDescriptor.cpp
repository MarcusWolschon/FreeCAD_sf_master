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

#ifndef _PreComp_
#endif

#include "TPGDescriptor.h"

namespace Cam {

	 TPGDescriptor::TPGDescriptor(QString id, QString name, QString description, QString type) {
        this->id = id;
        this->name = name;
        this->description = description;
        this->type = type;
        refcnt = 1;
    }
    /// Convenience method for using plain ascii string
    TPGDescriptor::TPGDescriptor(const char *id, const char * name, const char * description,
            const char * type) {
        this->id = QString::fromAscii(id);
        this->name = QString::fromAscii(name);
        this->description = QString::fromAscii(description);
        this->type = QString::fromAscii(type);
        refcnt = 1;
    }
    TPGDescriptor::TPGDescriptor(const TPGDescriptor &copy) {
        this->id = copy.id;
        this->name = copy.name;
        this->description = copy.description;
        this->type = copy.type;
        refcnt = 1;
    }
	TPGDescriptor::TPGDescriptor(const TPGDescriptor *copy) {
        this->id = copy->id;
        this->name = copy->name;
        this->description = copy->description;
        this->type = copy->type;
        refcnt = 1;
    }

	/* virtual */ TPGDescriptor::~TPGDescriptor() {
        qDebug("Deleted TPGDescriptor: %p\n", this);
    }

	/* virtual */ void TPGDescriptor::print() {
        qDebug("- ('%s', '%s', '%s', '%s')\n", id.toAscii().constData(),
                name.toAscii().constData(), description.toAscii().constData(),
                type.toAscii().constData());
    }






    /**
     * Increases reference count
     */
    TPGDescriptor *TPGDescriptor::grab() {
        refcnt++;

        return this;
    }

    /**
     * Decreases reference count and deletes self if no other references
     */
	void TPGDescriptor::release() {
        refcnt--;
        if (refcnt == 0)
            delete this;
    }

} /* namespace Cam */
