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

#ifndef PYTPGAPI_H_
#define PYTPGAPI_H_

#include <PreCompiled.h>
#ifndef _PreComp_
//# include <Python.h>
#endif

#include <vector>
#include <QString>

#include "TPGDescriptorCollection.h"
#include "TPGPython.h"
#include "TPGFactory.h"


namespace Cam {

/**
 * A class that manages the interface to the Python TPG implementations.
 */
class CamExport PyTPGFactoryInst
{
protected:
	static PyTPGFactoryInst* _pcSingleton;
	PyObject* obj;
	PyTPGFactoryInst();
	~PyTPGFactoryInst();

//	std::vector<TPGDescriptor *> tpgs;
    TPGDescriptorCollection* descriptors;


    QString PythonUCToQString(PyObject *obj);
    PyObject *QStringToPythonUC(const QString &str);

public:

	// singleton manipators
	static PyTPGFactoryInst& instance(void);
	static void destruct (void);

	/**
	 * Set the PyTPGFactory (python) callback object
	 */
	void setCallback(PyObject* obj);

	/**
	 * Gets the PyTPGFactory python class from the given module and sets it as the
	 * callback
	 */
	void loadCallbackFromModule(PyObject *mod);

	/// C++ API (to Python TPG's) ///
//	/**
//	 * Returns (by reference) the list of found Python Plugins.
//	 * This is a list of PythonTPGDescriptors
//	 *
//	 * TODO: make this a const return value.
//	 * @deprecated
//	 */
//	std::vector<TPGDescriptor*> &scanPlugins();

    /**
     * Searches for Python TPG's.
     */
    void scanPlugins();

    /**
     * Get a vector of all Python TPG's that are known about
     */
//    std::vector<TPGDescriptor*>* getDescriptors();
    TPGDescriptorCollection* getDescriptors();

	/**
	 * Gets a TPG given its id.
	 * @deprecated, use the descriptors make() method to get a plugin instance
	 */
	TPGPython *getPlugin(QString id);

	// testing
	void test();
	bool test1(PyObject* obj);
	bool pytest(PyObject* arg);
};

/// Get the global instance
inline PyTPGFactoryInst& PyTPGFactory(void)
{
    return PyTPGFactoryInst::instance();
}

} /* namespace Cam */
#endif /* PYTPGAPI_H_ */
