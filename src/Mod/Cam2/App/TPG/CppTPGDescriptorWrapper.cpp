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

#include "CppTPGDescriptorWrapper.h"

namespace Cam {

CppTPGDescriptorWrapper::CppTPGDescriptorWrapper(TPGDescriptor *descriptor, CppTPGPlugin *plugin/*=NULL*/)
: TPGDescriptor(descriptor) {

    if (descriptor == NULL)
	{
        qWarning("Warning: CppTPGDescriptorWrapper is wrapping NULL\n");
		this->descriptor = NULL;
	}
	else
	{
		this->descriptor = descriptor->grab();
	}

	if (plugin == NULL)
	{
		this->plugin = NULL;
	}
	else
	{
		this->plugin = plugin->grab();	// increment the reference counter
	}
}

CppTPGDescriptorWrapper::~CppTPGDescriptorWrapper() {
    if (descriptor != NULL)
	{
        descriptor->release();
		descriptor = NULL;
	}

	if (plugin != NULL)
	{
		plugin->release();	// decrement the reference counter
		plugin = NULL;
	}
}

/**
 * Creates a new instance of this TPG.  Sub-classes need to implement this
 */
TPG* CppTPGDescriptorWrapper::make()
{
    if (plugin != NULL)
        return plugin->getTPG(id);
    if (descriptor != NULL)
        return descriptor->make();  // This doesn't work on a CppTPGDescriptor but will work on the cached version later.
    return NULL;
}

} /* namespace CamGui */
