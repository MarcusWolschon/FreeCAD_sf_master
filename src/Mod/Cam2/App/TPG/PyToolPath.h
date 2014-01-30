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

#ifndef PYTOOLPATH_H_
#define PYTOOLPATH_H_

#include <Python.h>

#include "ToolPath.h"


/**
 * A wrapper for the ToolPath class to allow access from PyTPGs
 */
typedef struct  {
    PyObject_HEAD
    Cam::ToolPath *tp;
} cam_PyToolPath;


extern PyTypeObject *PyToolPath_Type();

/**
 * Wrapper function to create new instances of PyToolPath objects from c++
 */
extern PyObject* PyToolPath_New(Cam::ToolPath *tp);

#endif /* PYTOOLPATH_H_ */
