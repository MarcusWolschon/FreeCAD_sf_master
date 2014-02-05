/***************************************************************************
 *   Copyright (c) 2012 Andrew Robinson  (andrewjrobinson@gmail.com)       *
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

#include <qvector.h>
#include <Base/Interpreter.h>

#include "PyTPGFactory.h"
#include "PyTPGDescriptor.h"

namespace Cam {

PyTPGFactoryInst* PyTPGFactoryInst::_pcSingleton = NULL;

PyTPGFactoryInst& PyTPGFactoryInst::instance(void)
{
    if (_pcSingleton == NULL)
        _pcSingleton = new PyTPGFactoryInst();

    return *_pcSingleton;
}
void PyTPGFactoryInst::destruct (void)
{
    if (_pcSingleton != NULL)
        delete _pcSingleton;
    _pcSingleton = NULL;
}

PyTPGFactoryInst::PyTPGFactoryInst()
{
    Base::Interpreter().runString("import PyTPG");
    this->obj = NULL;
    descriptors = NULL;
}

/**
 * Cleanup the Python objects if set
 */
PyTPGFactoryInst::~PyTPGFactoryInst()
{
    if (this->obj != NULL) {
        Py_DecRef(this->obj);
        this->obj = NULL;
    }
}

///////////// Callback setters //////////////

/**
 * Saves the callback object for later use.
 * Note: Refcount is increased for both params when it is stored
 */
void PyTPGFactoryInst::setCallback(PyObject* obj)
{
    if (this->obj != NULL)
        Py_DecRef(this->obj);
    this->obj = obj;
    Py_IncRef(obj);
}

/**
 * Gets the PyTPGManager python class from the given module and sets it as the
 * callback
 */
void PyTPGFactoryInst::loadCallbackFromModule(PyObject *mod)
{
    PyObject *pyTPGFactorycls = PyObject_GetAttrString(mod, "PyTPGFactory");
    if (pyTPGFactorycls != NULL) {
        PyObject *obj = PyObject_CallMethod(pyTPGFactorycls, "instance", NULL);
        if (obj != NULL) {
            setCallback(obj);
            Py_DecRef(obj);
        }
        Py_DecRef(pyTPGFactorycls);
    }
}


///////// Testing methods ///////////////

void PyTPGFactoryInst::test()
{
    if (this->obj != NULL) {
        PyObject * result = PyObject_CallMethod(obj, "printPlugins", "");

        if (result != NULL)
          Py_DecRef(result);
    }
}

bool PyTPGFactoryInst::test1(PyObject* obj)
{
    QString str = this->PythonUCToQString(obj);
    printf("Test1((QString)'%s')\n", str.toStdString().c_str());
    PyObject* uc = this->QStringToPythonUC(str);
    return this->pytest(uc);
}


bool PyTPGFactoryInst::pytest(PyObject* arg)
{
    if (this->obj != NULL)
    {
        PyObject * result = PyObject_CallMethod(this->obj, "pytest", "(O)", arg);
        if (result != NULL) {
            Py_DecRef(result);
            return true;
        } else {
          if (PyErr_Occurred() == NULL) {
            printf("pytest: no error\n");
            return true;
          } else
            printf("pytest: error\n");
        }
    }

    return false;
}

//////////// C++ API (to Python TPG's) ///////////////

/**
 * Returns (by reference) the list of found Python Plugins.
 * This is a vector of PythonTPGDescriptors
 */
void PyTPGFactoryInst::scanPlugins() {
    if (this->obj != NULL) {
        PyGILState_STATE state = PyGILState_Ensure();
        PyObject * result = PyObject_CallMethod(obj, "scanPlugins", NULL);
        if (result != NULL) {
            if (PyList_Check(result)) {

                // clean the current list
                if (descriptors != NULL) {
		   descriptors->release();
		}
                descriptors = new TPGDescriptorCollection();

                // add the new items
                int len = PyList_Size(result);
                for (int i = 0; i < len; i++) {
                    PyObject *tuple = PyList_GetItem(result, i);
                    if (PyTuple_Check(tuple) && PyTuple_GET_SIZE(tuple) == 3) {
                        PyObject *pid = PyTuple_GET_ITEM(tuple, 0);
                        PyObject *pname = PyTuple_GET_ITEM(tuple, 1);
                        PyObject *pdesc = PyTuple_GET_ITEM(tuple, 2);

                          descriptors->add(new PyTPGDescriptor(PythonUCToQString(pid),
                                        PythonUCToQString(pname),
                                        PythonUCToQString(pdesc)));
                    }
                }
            }
            Py_DecRef(result);
        }
        PyGILState_Release(state);
    }
}
/**
 * Get a vector of all C++ TPG's that are known about
 */
TPGDescriptorCollection* PyTPGFactoryInst::getDescriptors()
{
    // cache a copy of the descriptors
    if (descriptors == NULL)
        this->scanPlugins();

    printf("Found %i PyTPGs\n", descriptors->size());

    // copy the tpg list cache
    return descriptors->clone();
}

/**
 * Gets a TPG given its id.
 *
 * Note: used by the PyTPGDecriptor to get the required PyTPG
 */
TPGPython *PyTPGFactoryInst::getPlugin(QString id)
{
    TPGPython *tpg = NULL;
    if (this->obj != NULL)
    {
        PyObject *arg    = QStringToPythonUC(id);
        PyGILState_STATE state = PyGILState_Ensure();

        // get the python class (TPG implementation)
        PyObject *result = PyObject_CallMethod(obj, "getPlugin", "(O)", arg);
        if (result != NULL) {
          if (PyType_Check(result)) {

            // wrap in C++ TPG
            tpg = new TPGPython(result);
            PyGILState_Release(state);
            return tpg;
          } else
              printf("ERROR Unable to get Python Class'%s'\n", Py_TYPE(result)->tp_name);
          Py_DecRef(result);
        }
        PyGILState_Release(state);
    }
    return tpg;
}

//////////// Support methods /////////////
//TODO: move to a separate CPP file
PyObject *PyTPGFactoryInst::QStringToPythonUC(const QString &str)
{
    //TODO: check this works properly, it appears to work with the UCS4 characters I tested
    return PyString_FromString((const char*)str.toStdString().c_str());
}

QString PyTPGFactoryInst::PythonUCToQString(PyObject *obj)
{
  QString conv;

  if (obj == Py_None) // None to ""
  {}
  else if (PyUnicode_Check(obj)) // Unicode values
  {
    PyObject* uc = PyUnicode_AsUTF8String(obj);
    if (uc != NULL) {
      conv = QString::fromUtf8(PyString_AS_STRING(uc));
      Py_DecRef(uc);
    }
    else
      conv = QString::fromAscii("Unicode Conversion Problem");
  }
  else if (PyString_Check(obj)) // String values
    conv = QString::fromUtf8(PyString_AS_STRING(obj));
  else
    conv = QString::fromAscii("Non-string object");
  return conv;
}

} /* namespace Cam */
