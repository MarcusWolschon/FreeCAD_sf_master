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

#include <PreCompiled.h>
#ifndef _PreComp_
#endif

#include <Python.h>

#include "PyToolPath.h"
#include "ToolPath.h"

//namespace Cam {

/**
 * The python deallocator
 */
static void
PyToolPath_dealloc(cam_PyToolPath* self) {
    if (self->tp != NULL) {
        self->tp->release();
        self->tp = NULL;
    }
    self->ob_type->tp_free((PyObject*)self);
}

/**
 * The python allocator
 */
static PyObject *
PyToolPath_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    cam_PyToolPath *self;

    self = (cam_PyToolPath *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->tp = new Cam::ToolPath(NULL);
    }

    return (PyObject *)self;
}

/**
 * Python initialiser
 */
static int
PyToolPath_init(cam_PyToolPath *self, PyObject *args, PyObject *kwds) {

    self->tp->clear();
    return 0;
}

/**
 * Clear the Toolpath
 */
static PyObject *
PyToolPath_clear(cam_PyToolPath* self) {

    self->tp->clear();

    Py_RETURN_NONE;
}

/**
 * Add a command to the toolpath
 */
static PyObject *
PyToolPath_addToolPath (cam_PyToolPath* self, PyObject* args) {

    const char *command;
    // int sts;

    if (!PyArg_ParseTuple(args, "s", &command)) {
        PyErr_SetString(PyExc_TypeError, "Single String or Unicode argument expected");
        return NULL;
    }

    self->tp->addToolPath(QString::fromAscii(command));

    Py_RETURN_NONE;
}

/**
 * Method table for PyToolPath python type
 */
static PyMethodDef PyToolPath_methods[] = {
    {"clear",       (PyCFunction)PyToolPath_clear,       METH_NOARGS,  "Empty the tool-path"},
    {"addToolPath", (PyCFunction)PyToolPath_addToolPath, METH_VARARGS, "Add a tool-path command to the tool-path"},
    {NULL}  /* Sentinel */
};

static PyTypeObject PyToolPathType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "cam.PyToolPath",             /*tp_name*/
    sizeof(cam_PyToolPath),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyToolPath_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Storage object for tool-paths",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    PyToolPath_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyToolPath_init,      /* tp_init */
    0,                         /* tp_alloc */
    PyToolPath_new,                 /* tp_new */
};


extern PyTypeObject *PyToolPath_Type() {
    return &PyToolPathType;
}


/**
 * Wrapper function to create new instances of PyToolPath objects from c++
 */
extern PyObject* PyToolPath_New(Cam::ToolPath *tp) {

    cam_PyToolPath *self;

    self = (cam_PyToolPath *)PyToolPathType.tp_alloc(&PyToolPathType, 0);
    if (self != NULL) {
        self->tp = tp->grab();
    }

    return (PyObject *)self;
}


//} /* namespace Cam */
