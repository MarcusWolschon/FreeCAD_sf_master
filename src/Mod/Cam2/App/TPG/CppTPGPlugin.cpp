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

#include "CppTPGPlugin.h"
#include "CppTPGDescriptorWrapper.h"

namespace Cam {

CppTPGPlugin::CppTPGPlugin(QString filename) : library(filename)
{
    this->filename = filename;
    getDescriptorsPtr = NULL;
    getTPGPtr = NULL;
    descriptors = NULL;
    refcnt = 1;
}

CppTPGPlugin::~CppTPGPlugin() {
    if (descriptors != NULL)
	{
        descriptors->release();
		descriptors = NULL;
	}

    if (library.isLoaded())
	{
        close();
	}

	getTPGPtr = NULL;
	getDescriptorsPtr = NULL;

	refcnt = 0;
}

/**
 * A callback function to wrap each element in a wrapper.
 */
void* wrapfunc(TPGDescriptor* tpg, void* plugin) {
    return new CppTPGDescriptorWrapper(tpg, (CppTPGPlugin*)plugin);
}

/**
 * Returns a list of TPG's that this plugin provides
 * Note: the list is owned by the caller and must be released when finished
 * with it (including the referenced TPGDescriptors).
 */
Cam::TPGDescriptorCollection* CppTPGPlugin::getDescriptors() {
    if (isOpen()) {
        if (descriptors == NULL) {
            descriptors = getDescriptorsPtr();
        }
        if (descriptors != NULL) {
            // wrap the descriptors
            Cam::TPGDescriptorCollection* result = descriptors->clone();
            result->onEach(wrapfunc, (void*)this);
            return result;
        }
    }
    qWarning("Warning: NULL descriptors: %s\nError: %s\n", this->filename.toAscii().constData(), this->error.toAscii().constData());
    return NULL;
}

/**
 * Gets an instance of the TPG.
 * Note: the TPG returned is a wrapper around the implementation.  Release
 * it once you are finished with it and it will automatically delete the
 * implementation once all references are released
 */
TPG* CppTPGPlugin::getTPG(QString id) {
    if ((isOpen()) && (getTPGPtr != NULL)) {
        return getTPGPtr(id);
    }
    qWarning("Warning: NULL descriptors: %s\nError: %s\n", this->filename.toAscii().constData(), this->error.toAscii().constData());
    return NULL;
}

/**
 * Increases reference count
 */
CppTPGPlugin* CppTPGPlugin::grab() {
    refcnt++;
    return this;
}

/**
 * Decreases reference count and deletes self if no other references
 */
void CppTPGPlugin::release() {
    refcnt--;
    if (refcnt <= 0)
	{
        delete this;
	}
}

/**
 * Makes sure library is open, attempts to open it if it isn't.
 */
bool CppTPGPlugin::isOpen() {
	if (! library.isLoaded()) {
        library.load();
		if (! library.isLoaded()) {
			error = library.errorString();
            return false;
        }

		getDescriptorsPtr = NULL;
		getTPGPtr = NULL;

        // resolve symbols
		getDescriptorsPtr = (getDescriptors_t*) library.resolve("getDescriptors");
		if (getDescriptorsPtr == NULL) {error = library.errorString(); close(); return false;}
		qDebug("Correctly retrieved function pointers for getDescriptors() from plugin library %s\n", this->filename.toAscii().constData() );	// TODO - remove this when the module is more robust.

		getTPGPtr = (getTPG_t*) library.resolve("getTPG");
		if (getTPGPtr == NULL) {error = library.errorString(); close(); return false;}
		printf("Correctly retrieved function pointer for getTPG() from plugin library %s\n", this->filename.toAscii().constData() );	// TODO - remove this when the module is more robust.
    }
    return true;
}

/**
 * Close the library and cleanup pointers
 */
void CppTPGPlugin::close() {
	if (library.isLoaded()) {
//        // clear caches
//        if (descriptors != NULL)
//            delDescriptorsPtr(descriptors);

		library.unload();
    }

	getDescriptorsPtr = NULL;
	getTPGPtr = NULL;
}

} /* namespace CamGui */
