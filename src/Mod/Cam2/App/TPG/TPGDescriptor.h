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

#ifndef TPGDESCRIPTOR2_H_
#define TPGDESCRIPTOR2_H_

#include <PreCompiled.h>

namespace Cam {
class CamExport TPGDescriptor;
}
#include <QString>

#include "TPG.h"

namespace Cam {

/**
 * A superclass for TPG Descriptors.  These describe the basic information
 * about a TPG and contain a method to create a new instance of this TPG.
 */
class CamExport TPGDescriptor {

protected:
    int refcnt;

    virtual ~TPGDescriptor();

public:
    QString id;
    QString name;
    QString description;

    QString type; // subclasses should set this to their classname in the constructor

    TPGDescriptor(QString id, QString name, QString description, QString type);

    /// Convenience method for using plain ascii string
    TPGDescriptor(const char *id, const char * name, const char * description, const char * type);
    TPGDescriptor(const TPGDescriptor &copy);
    TPGDescriptor(const TPGDescriptor *copy);

    /**
     * Increases reference count
     */
    TPGDescriptor *grab();

    /**
     * Decreases reference count and deletes self if no other references
     */
    void release();

    /**
     * Creates a new instance of this TPG.  Sub-classes need to implement this
     */
    virtual Cam::TPG* make() = 0;
    virtual void print();
};

} /* namespace Cam */
#endif /* TPGDESCRIPTOR2_H_ */
