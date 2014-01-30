/***************************************************************************
 *   Copyright (c) 2012 Luke Parry    (l.parry@warwick.ac.uk)              *
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

#include <App/Application.h>
#include <Base/Console.h>
#include <Base/Sequencer.h>

#include <Python.h>

#include <QList>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include "TPG.h"
#include "TPGPython.h"
#include "TPGFactory.h"
#include "PyTPGFactory.h"
#include "CppTPGFactory.h"

using namespace Cam;


TPGFactoryInst* TPGFactoryInst::_pcSingleton = NULL;

TPGFactoryInst& TPGFactoryInst::instance(void)
{
    if (_pcSingleton == NULL) {
        _pcSingleton = new TPGFactoryInst;
    }

    return *_pcSingleton;
}

TPGFactoryInst::TPGFactoryInst(void)
{
    d = new TPGFactoryInstP;
}

TPGFactoryInst::~TPGFactoryInst(void)
{
    delete d;
}

void TPGFactoryInst::destruct (void)
{
    if (_pcSingleton != 0)
        delete _pcSingleton;
    _pcSingleton = 0;
}

void TPGFactoryInst::clearDescriptors() {
    if (d->descriptors != NULL)
        d->descriptors->release();
    d->descriptors = new Cam::TPGDescriptorCollection();
}

TPG * TPGFactoryInst::getPlugin(QString id) {
    for(size_t i = 0; i < d->descriptors->size(); ++i) {
        TPGDescriptor *tpg = d->descriptors->at(i);
        if (tpg->id == id)
            return tpg->make();
    }

    return 0;
}

/**
 * Scans for TPG's and updates internal cache of known tpgs
 *
 * @see getDescriptors to get a copy of these.
 */
void TPGFactoryInst::scanPlugins(short tpgtype /*= ALL_TPG*/) {

    // scan for plugins
    if ((tpgtype & PYTHON_TPG) > 0)
        Cam::PyTPGFactory().scanPlugins();
    if ((tpgtype & CPP_TPG) > 0)
        Cam::CppTPGFactory().scanPlugins();

    // clear my cache (if any)
    clearDescriptors();

    // update cache
    Cam::TPGDescriptorCollection *descs = Cam::PyTPGFactory().getDescriptors();
	if (descs)
	{
		d->descriptors->absorb(descs);
		descs->release();
	}
    descs = Cam::CppTPGFactory().getDescriptors();
	if (descs)
	{
		d->descriptors->absorb(descs);
		descs->release();
	}
}

/**
 * Get a vector of all C++ TPG's that are known about
 */
Cam::TPGDescriptorCollection* TPGFactoryInst::getDescriptors()
{
    // cache a copy of the descriptors
    if (d->descriptors->size() == 0)
        this->scanPlugins();

    // copy the tpg list cache
    return d->descriptors->clone();
}
